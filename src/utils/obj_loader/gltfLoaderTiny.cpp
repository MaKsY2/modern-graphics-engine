#include "gltfLoaderTiny.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstring> // TODO: убрать пидорство для memcpy
#include <stdexcept>
#include <iostream>
#include <unordered_set>

namespace loader
{
    namespace
    {
        inline float toFloat(double v) { return static_cast<float>(v); }

        glm::mat4 nodeLocalMatrix(const tinygltf::Node &n)
        {
            if (n.matrix.size() == 16)
            {
                glm::mat4 M(1.0f);
                for (int c = 0; c < 4; ++c)
                    for (int r = 0; r < 4; ++r)
                        M[c][r] = toFloat(n.matrix[static_cast<size_t>(c) * 4 + r]);
                return M;
            }

            glm::vec3 t(0.0f), s(1.0f);
            glm::quat q(1.0f, 0.0f, 0.0f, 0.0f); // w,x,y,z

            if (n.translation.size() == 3)
                t = {toFloat(n.translation[0]), toFloat(n.translation[1]), toFloat(n.translation[2])};
            if (n.scale.size() == 3)
                s = {toFloat(n.scale[0]), toFloat(n.scale[1]), toFloat(n.scale[2])};
            if (n.rotation.size() == 4)
            {
                // glTF: x y z w
                q = glm::quat(toFloat(n.rotation[3]),
                              toFloat(n.rotation[0]),
                              toFloat(n.rotation[1]),
                              toFloat(n.rotation[2]));
            }

            return glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(q) * glm::scale(glm::mat4(1.0f), s);
        }

        const std::byte *accessorBasePtr(const tinygltf::Model &model, const tinygltf::Accessor &acc)
        {
            if (acc.bufferView < 0)
                throw std::runtime_error("Accessor has no bufferView");
            const auto &view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
            const auto &buf = model.buffers[static_cast<size_t>(view.buffer)];
            const size_t offset = static_cast<size_t>(view.byteOffset) + static_cast<size_t>(acc.byteOffset);
            if (offset >= buf.data.size())
                throw std::runtime_error("Accessor offset out of range");
            return reinterpret_cast<const std::byte *>(buf.data.data() + offset);
        }

        size_t accessorStride(const tinygltf::Model &model, const tinygltf::Accessor &acc)
        {
            const auto &view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
            const size_t s = acc.ByteStride(view);
            if (s != 0)
                return s;

            const int comps = tinygltf::GetNumComponentsInType(acc.type);
            const int compSize = tinygltf::GetComponentSizeInBytes(acc.componentType);
            if (comps <= 0 || compSize <= 0)
                throw std::runtime_error("Bad accessor layout");
            return static_cast<size_t>(comps * compSize);
        }

        glm::vec3 readVec3Float(const std::byte *base, size_t stride, size_t index)
        {
            float tmp[3]{};
            std::memcpy(tmp, base + stride * index, sizeof(tmp));
            return {tmp[0], tmp[1], tmp[2]};
        }

        glm::vec2 readVec2Float(const std::byte *base, size_t stride, size_t index)
        {
            float tmp[2]{};
            std::memcpy(tmp, base + stride * index, sizeof(tmp));
            return {tmp[0], tmp[1]};
        }

        std::uint32_t readIndexScalar(const std::byte *base, size_t stride, size_t index, int componentType)
        {
            const std::byte *p = base + stride * index;

            switch (componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            {
                std::uint8_t v{};
                std::memcpy(&v, p, sizeof(v));
                return static_cast<std::uint32_t>(v);
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            {
                std::uint16_t v{};
                std::memcpy(&v, p, sizeof(v));
                return static_cast<std::uint32_t>(v);
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            {
                std::uint32_t v{};
                std::memcpy(&v, p, sizeof(v));
                return v;
            }
            default:
                throw std::runtime_error("Unsupported index component type");
            }
        }

        void appendPrimitivePU(const tinygltf::Model &model,
                               const tinygltf::Primitive &prim,
                               const glm::mat4 &world,
                               LoadedMeshPU &out,
                               bool flipV)
        {
            if (prim.mode != TINYGLTF_MODE_TRIANGLES)
                return;

            const auto itPos = prim.attributes.find("POSITION");
            if (itPos == prim.attributes.end())
                return;

            const tinygltf::Accessor &accPos = model.accessors[static_cast<size_t>(itPos->second)];

            const tinygltf::Accessor *accUv = nullptr;
            const auto itUv = prim.attributes.find("TEXCOORD_0");
            if (itUv != prim.attributes.end())
                accUv = &model.accessors[static_cast<size_t>(itUv->second)];

            // Минимально: ждём float vec3 позиции, float vec2 uv (если есть)
            if (accPos.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accPos.type != TINYGLTF_TYPE_VEC3)
                throw std::runtime_error("POSITION must be FLOAT VEC3");

            if (accUv)
            {
                if (accUv->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || accUv->type != TINYGLTF_TYPE_VEC2)
                    throw std::runtime_error("TEXCOORD_0 must be FLOAT VEC2 (minimal loader)");
            }

            const std::byte *posBase = accessorBasePtr(model, accPos);
            const size_t posStride = accessorStride(model, accPos);

            const std::byte *uvBase = nullptr;
            size_t uvStride = 0;
            if (accUv)
            {
                uvBase = accessorBasePtr(model, *accUv);
                uvStride = accessorStride(model, *accUv);
            }

            const std::uint32_t baseVertex = static_cast<std::uint32_t>(out.vertices.size());
            out.vertices.resize(out.vertices.size() + accPos.count);

            for (size_t i = 0; i < accPos.count; ++i)
            {
                const glm::vec3 P = readVec3Float(posBase, posStride, i);
                const glm::vec3 Pw = glm::vec3(world * glm::vec4(P, 1.0f));

                glm::vec2 uv(0.0f);
                if (uvBase)
                {
                    uv = readVec2Float(uvBase, uvStride, i);
                    if (flipV)
                        uv.y = 1.0f - uv.y; // часто нужно для OpenGL
                }

                out.vertices[baseVertex + static_cast<std::uint32_t>(i)] = {Pw, uv};
            }

            // indices
            if (prim.indices < 0)
            {
                out.indices.reserve(out.indices.size() + accPos.count);
                for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(accPos.count); ++i)
                    out.indices.push_back(baseVertex + i);
                return;
            }

            const tinygltf::Accessor &accIdx = model.accessors[static_cast<size_t>(prim.indices)];
            if (accIdx.type != TINYGLTF_TYPE_SCALAR)
                throw std::runtime_error("Indices accessor must be SCALAR");

            const std::byte *idxBase = accessorBasePtr(model, accIdx);
            const size_t idxStride = accessorStride(model, accIdx);

            out.indices.reserve(out.indices.size() + accIdx.count);
            for (size_t k = 0; k < accIdx.count; ++k)
            {
                const std::uint32_t idx = readIndexScalar(idxBase, idxStride, k, accIdx.componentType);
                out.indices.push_back(baseVertex + idx);
            }
        }

        void processNodePU(const tinygltf::Model &model,
                           int nodeIndex,
                           const glm::mat4 &parent,
                           LoadedMeshPU &out,
                           bool flipV)
        {
            const auto &node = model.nodes[static_cast<size_t>(nodeIndex)];
            const glm::mat4 world = parent * nodeLocalMatrix(node);

            if (node.mesh >= 0)
            {
                const auto &mesh = model.meshes[static_cast<size_t>(node.mesh)];
                for (const auto &prim : mesh.primitives)
                    appendPrimitivePU(model, prim, world, out, flipV);
            }

            for (int child : node.children)
                processNodePU(model, child, world, out, flipV);
        }
    } // namespace

    LoadedMeshPU LoadGLB_ToCPU_PU(const std::string &path, bool flipV)
    {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err, warn;

        const bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, path);
        if (!ok)
            throw std::runtime_error("LoadGLB failed: " + err);

        const int sceneIndex = (model.defaultScene >= 0) ? model.defaultScene : 0;
        if (sceneIndex < 0 || sceneIndex >= static_cast<int>(model.scenes.size()))
            throw std::runtime_error("No valid scene in GLB");

        LoadedMeshPU out;

        const auto &scene = model.scenes[static_cast<size_t>(sceneIndex)];
        for (int n : scene.nodes)
            processNodePU(model, n, glm::mat4(1.0f), out, flipV);

        if (out.vertices.empty() || out.indices.empty())
            throw std::runtime_error("No geometry found in GLB");

        return out;
    }
}