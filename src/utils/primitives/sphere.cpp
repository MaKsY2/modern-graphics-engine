#include "sphere.hpp"

#include <vector>

#include "mesh.hpp"
#include "shader.hpp"

namespace primitives {

namespace {
struct Entry {
  uint32_t st, sl;
  std::shared_ptr<utils::Mesh> mesh;
};

std::shared_ptr<utils::Mesh> createUnitSphereMesh(uint32_t stacks,
                                                  uint32_t slices) {
  std::vector<utils::VertexPU> v;
  std::vector<uint32_t> idx;

  v.reserve((stacks + 1) * (slices + 1));

  for (uint32_t y = 0; y <= stacks; ++y) {
    float vy = static_cast<float>(y) / static_cast<float>(stacks);
    float phi = vy * static_cast<float>(M_PI);

    for (uint32_t x = 0; x <= slices; ++x) {
      float vx = static_cast<float>(x) / static_cast<float>(slices);
      float theta = vx * static_cast<float>(M_PI * 2.0f);

      float sx = std::sin(phi) * std::cos(theta);
      float sy = std::cos(phi);
      float sz = std::sin(phi) * std::sin(theta);

      utils::VertexPU vert;
      vert.pos = glm::vec3(sx, sy, sz) * 0.5f;
      vert.uv = glm::vec2(vx, 1.0f - vy);
      v.push_back(vert);
    }
  }

  for (int y = 0; y < stacks; ++y) {
    for (int x = 0; x < slices; ++x) {
      uint32_t i0 = y * (slices + 1) + x;
      uint32_t i1 = i0 + 1;
      uint32_t i2 = i0 + (slices + 1);
      uint32_t i3 = i2 + 1;

      idx.push_back(i0);
      idx.push_back(i2);
      idx.push_back(i1);
      idx.push_back(i1);
      idx.push_back(i2);
      idx.push_back(i3);
    }
  }

  auto m = std::make_shared<utils::Mesh>();
  m->upload(v, idx);
  return m;
}
}  // namespace

std::shared_ptr<utils::Mesh> Sphere::getCachedMesh(uint32_t stacks,
                                                   uint32_t slices) {
  std::vector<Entry> cache;

  for (auto& e : cache)
    if (e.st == stacks && e.sl == slices) return e.mesh;

  auto mesh = createUnitSphereMesh(stacks, slices);
  cache.push_back({stacks, slices, mesh});
  return mesh;
}

Sphere::Sphere(float radius, uint32_t stacks, uint32_t slices,
               const std::shared_ptr<Shader>& shader)
    : utils::RenderObject(getCachedMesh(stacks, slices), shader) {
  transform.scale = glm::vec3(radius * 2.0f);
  transform.dirty = true;
}
}  // namespace primitives
