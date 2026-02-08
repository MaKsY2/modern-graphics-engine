#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <vector>

namespace utils {

struct VertexPU {
  glm::vec3 pos;
  glm::vec2 uv;
};

class Mesh {
 public:
  ~Mesh();

  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&&);

  Mesh(const Mesh&) = delete;
  Mesh(Mesh&&) noexcept;
  Mesh() = default;

  void upload(const std::vector<VertexPU>& vertices,
              const std::vector<uint32_t>& indices = {});
  void draw(GLenum prim = GL_TRIANGLES) const;

 private:
  void destroy();

  void moveFrom(Mesh&& o);

  GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
  GLsizei vertexCount_ = 0;
  GLsizei indexCount_ = 0;
  bool indexed_ = false;
};

}  // namespace utils

#endif