#include "gltfLoaderTiny.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

#include "../gl_debug.hpp"
#include "tiny_gltf.h"

namespace loader {
namespace {

inline float f(double v) { return static_cast<float>(v); }

glm::mat4 nodeLocalMatrix(const tinygltf::Node& n) {
  if (n.matrix.size() == 16) {
    glm::mat4 M(1.0f);
    for (int c = 0; c < 4; ++c)
      for (int r = 0; r < 4; ++r)
        M[c][r] = f(n.matrix[static_cast<size_t>(c) * 4 + r]);
    return M;
  }

  glm::vec3 t(0.0f), s(1.0f);
  glm::quat q(1.0f, 0.0f, 0.0f, 0.0f);  // w,x,y,z

  if (n.translation.size() == 3)
    t = {f(n.translation[0]), f(n.translation[1]), f(n.translation[2])};
  if (n.scale.size() == 3) s = {f(n.scale[0]), f(n.scale[1]), f(n.scale[2])};
  if (n.rotation.size() == 4)
    q = glm::quat(f(n.rotation[3]), f(n.rotation[0]), f(n.rotation[1]),
                  f(n.rotation[2]));

  return glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(q) *
         glm::scale(glm::mat4(1.0f), s);
}

const std::byte* accBasePtr(const tinygltf::Model& model,
                            const tinygltf::Accessor& acc) {
  if (acc.bufferView < 0)
    throw std::runtime_error("Accessor has no bufferView");
  const auto& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
  const auto& buf = model.buffers[static_cast<size_t>(view.buffer)];
  const size_t off = static_cast<size_t>(view.byteOffset) +
                     static_cast<size_t>(acc.byteOffset);
  if (off >= buf.data.size())
    throw std::runtime_error("Accessor offset out of range");
  return reinterpret_cast<const std::byte*>(buf.data.data() + off);
}

size_t accStride(const tinygltf::Model& model, const tinygltf::Accessor& acc) {
  const auto& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
  size_t s = acc.ByteStride(view);
  if (s != 0) return s;

  const int comps = tinygltf::GetNumComponentsInType(acc.type);
  const int compSize = tinygltf::GetComponentSizeInBytes(acc.componentType);
  if (comps <= 0 || compSize <= 0)
    throw std::runtime_error("Bad accessor layout");
  return static_cast<size_t>(comps * compSize);
}

glm::vec3 readVec3f(const std::byte* base, size_t stride, size_t i) {
  float tmp[3]{};
  std::memcpy(tmp, base + stride * i, sizeof(tmp));
  return {tmp[0], tmp[1], tmp[2]};
}

glm::vec2 readVec2f(const std::byte* base, size_t stride, size_t i) {
  float tmp[2]{};
  std::memcpy(tmp, base + stride * i, sizeof(tmp));
  return {tmp[0], tmp[1]};
}

std::uint32_t readIndex(const std::byte* base, size_t stride, size_t i,
                        int componentType) {
  const std::byte* p = base + stride * i;
  switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
      std::uint8_t v{};
      std::memcpy(&v, p, sizeof(v));
      return v;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
      std::uint16_t v{};
      std::memcpy(&v, p, sizeof(v));
      return v;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
      std::uint32_t v{};
      std::memcpy(&v, p, sizeof(v));
      return v;
    }
    default:
      throw std::runtime_error("Unsupported index componentType");
  }
}

void computeNormalsRange(std::vector<utils::VertexPU>& v,
                         const std::vector<std::uint32_t>& idx,
                         std::uint32_t start, std::uint32_t count) {
  for (std::uint32_t i = start; i < start + count; ++i) {
    v[idx[i]].normal = glm::vec3(0.0f);
  }

  for (std::uint32_t i = start; i + 2 < start + count; i += 3) {
    auto i0 = idx[i];
    auto i1 = idx[i + 1];
    auto i2 = idx[i + 2];
    glm::vec3 n = glm::normalize(
        glm::cross(v[i1].pos - v[i0].pos, v[i2].pos - v[i0].pos));
    v[i0].normal += n;
    v[i1].normal += n;
    v[i2].normal += n;
  }

  for (std::uint32_t i = start; i < start + count; ++i) {
    auto vi = idx[i];
    float len = glm::length(v[vi].normal);
    v[vi].normal = (len > 1e-8f) ? (v[vi].normal / len) : glm::vec3(0, 1, 0);
  }
}

GLint toGLWrap(int wrap) {
  switch (wrap) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      return GL_REPEAT;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      return GL_CLAMP_TO_EDGE;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      return GL_MIRRORED_REPEAT;
    default:
      return GL_REPEAT;
  }
}

GLint toGLMinFilter(int f) {
  switch (f) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      return GL_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      return GL_LINEAR;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
      return GL_NEAREST_MIPMAP_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      return GL_LINEAR_MIPMAP_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      return GL_NEAREST_MIPMAP_LINEAR;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      return GL_LINEAR_MIPMAP_LINEAR;
    default:
      return GL_LINEAR_MIPMAP_LINEAR;
  }
}

GLint toGLMagFilter(int f) {
  switch (f) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      return GL_NEAREST;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      return GL_LINEAR;
    default:
      return GL_LINEAR;
  }
}

GLuint createTextureFromImage(const tinygltf::Image& img,
                              const tinygltf::Sampler* samplerOrNull,
                              bool srgbForBaseColor) {
  if (img.image.empty() || img.width <= 0 || img.height <= 0)
    throw std::runtime_error("Empty glTF image");

  GLenum format = GL_RGBA;
  if (img.component == 1)
    format = GL_RED;
  else if (img.component == 2)
    format = GL_RG;
  else if (img.component == 3)
    format = GL_RGB;
  else if (img.component == 4)
    format = GL_RGBA;
  else
    throw std::runtime_error("Unsupported image component count");

  GLenum internalFormat = GL_RGBA8;
  if (srgbForBaseColor) {
    if (format == GL_RGB)
      internalFormat = GL_SRGB8;
    else if (format == GL_RGBA)
      internalFormat = GL_SRGB8_ALPHA8;
    else
      internalFormat = GL_RGBA8;
  } else {
    if (format == GL_RGB)
      internalFormat = GL_RGB8;
    else if (format == GL_RGBA)
      internalFormat = GL_RGBA8;
  }

  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  GLint wrapS = GL_REPEAT, wrapT = GL_REPEAT;
  GLint minF = GL_LINEAR_MIPMAP_LINEAR, magF = GL_LINEAR;
  if (samplerOrNull) {
    wrapS = toGLWrap(samplerOrNull->wrapS);
    wrapT = toGLWrap(samplerOrNull->wrapT);
    minF = toGLMinFilter(samplerOrNull->minFilter);
    magF = toGLMagFilter(samplerOrNull->magFilter);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.width, img.height, 0,
               format, GL_UNSIGNED_BYTE, img.image.data());

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

utils::MaterialGL readMaterialAndCreateGL(
    const tinygltf::Model& model, int materialIndex,
    std::unordered_map<int, GLuint>& imageTexCache) {
  utils::MaterialGL out{};

  if (materialIndex < 0 ||
      materialIndex >= static_cast<int>(model.materials.size()))
    return out;

  const auto& mat = model.materials[static_cast<size_t>(materialIndex)];
  const auto& pbr = mat.pbrMetallicRoughness;

  if (pbr.baseColorFactor.size() == 4) {
    out.baseColorFactor =
        glm::vec4(f(pbr.baseColorFactor[0]), f(pbr.baseColorFactor[1]),
                  f(pbr.baseColorFactor[2]), f(pbr.baseColorFactor[3]));

    std::cout << "Material " << materialIndex
              << " baseColorFactor: " << out.baseColorFactor.r << ", "
              << out.baseColorFactor.g << ", " << out.baseColorFactor.b << ", "
              << out.baseColorFactor.a << std::endl;
  }
  const int texIndex = pbr.baseColorTexture.index;
  if (texIndex < 0 || texIndex >= static_cast<int>(model.textures.size()))
    return out;

  const auto& tex = model.textures[static_cast<size_t>(texIndex)];
  const int imageIndex = tex.source;
  if (imageIndex < 0 || imageIndex >= static_cast<int>(model.images.size()))
    return out;

  if (auto it = imageTexCache.find(imageIndex); it != imageTexCache.end()) {
    out.baseColorTex = it->second;
    out.hasBaseColorTex = (out.baseColorTex != 0);
    return out;
  }

  const tinygltf::Sampler* samplerPtr = nullptr;
  if (tex.sampler >= 0 && tex.sampler < static_cast<int>(model.samplers.size()))
    samplerPtr = &model.samplers[static_cast<size_t>(tex.sampler)];

  const auto& img = model.images[static_cast<size_t>(imageIndex)];
  out.baseColorTex =
      createTextureFromImage(img, samplerPtr, /*srgbForBaseColor=*/true);
  out.hasBaseColorTex = (out.baseColorTex != 0);

  imageTexCache[imageIndex] = out.baseColorTex;
  return out;
}

void appendPrimitive(const tinygltf::Model& model,
                     const tinygltf::Primitive& prim, const glm::mat4& world,
                     utils::ModelData& out, utils::Submesh& outSubmesh,
                     bool& normalsMissing) {
  if (prim.mode != TINYGLTF_MODE_TRIANGLES) return;

  const auto itPos = prim.attributes.find("POSITION");
  if (itPos == prim.attributes.end()) return;

  const tinygltf::Accessor& accPos =
      model.accessors[static_cast<size_t>(itPos->second)];

  const tinygltf::Accessor* accNor = nullptr;
  const auto itNor = prim.attributes.find("NORMAL");
  if (itNor != prim.attributes.end())
    accNor = &model.accessors[static_cast<size_t>(itNor->second)];
  else
    normalsMissing = true;

  const tinygltf::Accessor* accUV = nullptr;
  const auto itUV = prim.attributes.find("TEXCOORD_0");
  if (itUV != prim.attributes.end())
    accUV = &model.accessors[static_cast<size_t>(itUV->second)];

  if (accPos.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
      accPos.type != TINYGLTF_TYPE_VEC3)
    throw std::runtime_error("POSITION must be FLOAT VEC3");
  if (accNor && !(accNor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
                  accNor->type == TINYGLTF_TYPE_VEC3))
    throw std::runtime_error("NORMAL must be FLOAT VEC3");
  if (accUV && !(accUV->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
                 accUV->type == TINYGLTF_TYPE_VEC2))
    throw std::runtime_error("TEXCOORD_0 must be FLOAT VEC2 (minimal)");

  const std::byte* posBase = accBasePtr(model, accPos);
  const size_t posStride = accStride(model, accPos);

  const std::byte* norBase = nullptr;
  size_t norStride = 0;
  if (accNor) {
    norBase = accBasePtr(model, *accNor);
    norStride = accStride(model, *accNor);
  }

  const std::byte* uvBase = nullptr;
  size_t uvStride = 0;
  if (accUV) {
    uvBase = accBasePtr(model, *accUV);
    uvStride = accStride(model, *accUV);
  }

  const std::uint32_t baseVertex =
      static_cast<std::uint32_t>(out.vertices.size());
  out.vertices.resize(out.vertices.size() + accPos.count);

  const glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(world)));

  for (size_t i = 0; i < accPos.count; ++i) {
    glm::vec3 P = readVec3f(posBase, posStride, i);
    glm::vec3 Pw = glm::vec3(world * glm::vec4(P, 1.0f));

    glm::vec3 N(0, 1, 0);
    if (norBase) {
      glm::vec3 n = readVec3f(norBase, norStride, i);
      N = glm::normalize(normalMat * n);
    }

    glm::vec2 UV(0.0f);
    if (uvBase) UV = readVec2f(uvBase, uvStride, i);

    out.vertices[baseVertex + static_cast<std::uint32_t>(i)] = {Pw, UV, N};
  }
  outSubmesh.indexOffset = static_cast<std::uint32_t>(out.indices.size());
  outSubmesh.materialIndex = prim.material;

  if (prim.indices < 0) {
    out.indices.reserve(out.indices.size() + accPos.count);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(accPos.count); ++i)
      out.indices.push_back(baseVertex + i);
    outSubmesh.indexCount = static_cast<std::uint32_t>(accPos.count);
    return;
  }

  const tinygltf::Accessor& accIdx =
      model.accessors[static_cast<size_t>(prim.indices)];
  if (accIdx.type != TINYGLTF_TYPE_SCALAR)
    throw std::runtime_error("Indices accessor must be SCALAR");

  const std::byte* idxBase = accBasePtr(model, accIdx);
  const size_t idxStride = accStride(model, accIdx);

  out.indices.reserve(out.indices.size() + accIdx.count);
  for (size_t k = 0; k < accIdx.count; ++k) {
    std::uint32_t ix = readIndex(idxBase, idxStride, k, accIdx.componentType);
    out.indices.push_back(baseVertex + ix);
  }
  outSubmesh.indexCount = static_cast<std::uint32_t>(accIdx.count);
}

void processNode(const tinygltf::Model& model, int nodeIndex,
                 const glm::mat4& parent, utils::ModelData& out,
                 bool& anyMissingNormals,
                 const std::unordered_map<int, int>& materialRemap) {
  const auto& node = model.nodes[static_cast<size_t>(nodeIndex)];
  const glm::mat4 world = parent * nodeLocalMatrix(node);

  if (node.mesh >= 0) {
    const auto& mesh = model.meshes[static_cast<size_t>(node.mesh)];
    for (const auto& prim : mesh.primitives) {
      utils::Submesh sm{};
      bool normalsMissingThisPrim = false;

      appendPrimitive(model, prim, world, out, sm, normalsMissingThisPrim);

      if (sm.indexCount > 0) {
        if (sm.materialIndex >= 0) {
          if (auto it = materialRemap.find(sm.materialIndex);
              it != materialRemap.end())
            sm.materialIndex = it->second;
        }
        out.submeshes.push_back(sm);

        if (normalsMissingThisPrim) {
          anyMissingNormals = true;
          computeNormalsRange(out.vertices, out.indices, sm.indexOffset,
                              sm.indexCount);
        }
      }
    }
  }

  for (int child : node.children)
    processNode(model, child, world, out, anyMissingNormals, materialRemap);
}

}  // namespace

utils::ModelData LoadGLB_ToCPU(const std::string& path) {
  tinygltf::TinyGLTF loader;
  tinygltf::Model model;
  std::string err, warn;

  if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
    throw std::runtime_error("LoadGLB failed: " + err);

  const int sceneIndex = (model.defaultScene >= 0) ? model.defaultScene : 0;
  if (sceneIndex < 0 || sceneIndex >= static_cast<int>(model.scenes.size()))
    throw std::runtime_error("No valid scene in GLB");

  utils::ModelData out;

  out.materials.resize(model.materials.size());
  std::unordered_map<int, GLuint> imageTexCache;
  for (int mi = 0; mi < static_cast<int>(model.materials.size()); ++mi) {
    out.materials[static_cast<size_t>(mi)] =
        readMaterialAndCreateGL(model, mi, imageTexCache);
  }

  std::unordered_map<int, int> materialRemap;
  for (int mi = 0; mi < static_cast<int>(out.materials.size()); ++mi)
    materialRemap[mi] = mi;

  bool anyMissingNormals = false;

  const auto& scene = model.scenes[static_cast<size_t>(sceneIndex)];
  for (int n : scene.nodes)
    processNode(model, n, glm::mat4(1.0f), out, anyMissingNormals,
                materialRemap);

  if (out.vertices.empty() || out.indices.empty() || out.submeshes.empty())
    throw std::runtime_error("No geometry found in GLB");

  return out;
}

void DestroyModelTextures(utils::ModelData& m) {
  for (auto& mat : m.materials) {
    if (mat.baseColorTex != 0) {
      glDeleteTextures(1, &mat.baseColorTex);
      mat.baseColorTex = 0;
      mat.hasBaseColorTex = false;
    }
  }
}
}  // namespace loader