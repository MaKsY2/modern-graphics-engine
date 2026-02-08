#include "cube.hpp"

#include <vector>

#include "mesh.hpp"
#include "shader.hpp"

namespace primitives {

namespace {
const std::shared_ptr<utils::Mesh>& sharedUnitCubeMesh() {
  static std::shared_ptr<utils::Mesh> mesh = [] {
    std::vector<utils::VertexPU> v = {
        // +Z (front)
        {{-0.5f, -0.5f, 0.5f}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 1}},
        // -Z (back)
        {{0.5f, -0.5f, -0.5f}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {1, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 1}},
        // -X (left)
        {{-0.5f, -0.5f, -0.5f}, {0, 0}},
        {{-0.5f, -0.5f, 0.5f}, {1, 0}},
        {{-0.5f, 0.5f, 0.5f}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1}},
        // +X (right)
        {{0.5f, -0.5f, 0.5f}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {0, 1}},
        // +Y (top)
        {{-0.5f, 0.5f, 0.5f}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1}},
        // -Y (bottom)
        {{-0.5f, -0.5f, -0.5f}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {1, 1}},
        {{-0.5f, -0.5f, 0.5f}, {0, 1}},
    };

    std::vector<uint32_t> i;
    i.reserve(36);
    for (uint32_t f = 0; f < 6; ++f) {
      uint32_t base = f * 4;
      i.push_back(base + 0);
      i.push_back(base + 1);
      i.push_back(base + 2);
      i.push_back(base + 2);
      i.push_back(base + 3);
      i.push_back(base + 0);
    }

    auto m = std::make_shared<utils::Mesh>();
    m->upload(v, i);
    return m;
  }();
  return mesh;
}
}  // namespace
Cube::Cube(const glm::vec3& size, const std::shared_ptr<Shader>& shader)
    : utils::RenderObject(sharedUnitCubeMesh(), shader) {
  transform.scale = size;
  transform.dirty = true;
}
}  // namespace primitives
