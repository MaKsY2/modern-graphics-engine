#include "mesh.hpp"

namespace utils {

Mesh::~Mesh() { destroy(); }

Mesh::Mesh(Mesh&& other) noexcept { moveFrom(std::move(other)); }

void Mesh::destroy() {
  if (ebo_) glDeleteBuffers(1, &ebo_);
  if (vbo_) glDeleteBuffers(1, &vbo_);
  if (vao_) glDeleteVertexArrays(1, &vao_);
  vao_ = vbo_ = ebo_ = 0;
  vertexCount_ = indexCount_ = 0;
  indexed_ = false;
}

void Mesh::moveFrom(Mesh&& mesh) {
  vao_ = mesh.vao_;
  vbo_ = mesh.vbo_;
  ebo_ = mesh.ebo_;
  vertexCount_ = mesh.vertexCount_;
  indexCount_ = mesh.indexCount_;
  indexed_ = mesh.indexed_;
  mesh.vao_ = mesh.vbo_ = mesh.ebo_ = 0;
  mesh.vertexCount_ = mesh.indexCount_ = 0;
  mesh.indexed_ = false;
}

void Mesh::upload(const std::vector<VertexPU>& vertices,
                  const std::vector<uint32_t>& indices) {
  destroy();
  vertexCount_ = static_cast<GLsizei>(vertices.size());
  indexCount_ = static_cast<GLsizei>(indices.size());
  indexed_ = !indices.empty();

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPU),
               vertices.data(), GL_STATIC_DRAW);

  if (indexed_) {
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(VertexPU),
                 indices.data(), GL_STATIC_DRAW);
  }

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPU),
                        (void*)offsetof(VertexPU, pos));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPU),
                        (void*)offsetof(VertexPU, uv));

  glBindVertexArray(0);
}

void Mesh::draw(GLenum prim) const {
  glBindVertexArray(vao_);
  if (indexed_)
    glDrawElements(prim, indexCount_, GL_UNSIGNED_INT, nullptr);
  else
    glDrawArrays(prim, 0, vertexCount_);

  glBindVertexArray(0);
}
}  // namespace utils
