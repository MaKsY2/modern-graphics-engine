#include "mesh.hpp"

#include "gl_debug.hpp"

namespace utils {

VertexPU::VertexPU(glm::vec3 pos, glm::vec2 uv)
    : pos(pos), uv(uv), normal(glm::vec3{}) {}

VertexPU::VertexPU(glm::vec3 pos, glm::vec2 uv, glm::vec3 normal)
    : pos(pos), uv(uv), normal(normal) {}

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
  LOG_INFO("Mesh::upload - vertices: " << vertices.size()
                                       << ", indices: " << indices.size());

  if (vertices.empty()) {
    LOG_ERROR("Mesh::upload - vertices array is empty!");
    return;
  }

  destroy();
  vertexCount_ = static_cast<GLsizei>(vertices.size());
  indexCount_ = static_cast<GLsizei>(indices.size());
  indexed_ = !indices.empty();

  GL_CHECK(glGenVertexArrays(1, &vao_));
  LOG_INFO("Mesh::upload - VAO created: " << vao_);
  GL_CHECK(glBindVertexArray(vao_));

  GL_CHECK(glGenBuffers(1, &vbo_));
  LOG_INFO("Mesh::upload - VBO created: " << vbo_);
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo_));

  const size_t vboSize = vertices.size() * sizeof(VertexPU);
  LOG_INFO("Mesh::upload - VBO size: " << vboSize << " bytes");
  GL_CHECK(
      glBufferData(GL_ARRAY_BUFFER, vboSize, vertices.data(), GL_STATIC_DRAW));

  if (indexed_) {
    GL_CHECK(glGenBuffers(1, &ebo_));
    LOG_INFO("Mesh::upload - EBO created: " << ebo_);
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));

    const size_t eboSize = indices.size() * sizeof(uint32_t);
    LOG_INFO("Mesh::upload - EBO size: " << eboSize << " bytes (uint32_t size: "
                                         << sizeof(uint32_t) << ")");
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize, indices.data(),
                          GL_STATIC_DRAW));
  }

  GL_CHECK(glEnableVertexAttribArray(0));
  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPU),
                                 (void*)offsetof(VertexPU, pos)));

  GL_CHECK(glEnableVertexAttribArray(1));
  GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPU),
                                 (void*)offsetof(VertexPU, uv)));

  GL_CHECK(glEnableVertexAttribArray(2));
  GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPU),
                                 (void*)offsetof(VertexPU, normal)));

  GL_CHECK(glBindVertexArray(0));
  LOG_INFO("Mesh::upload - completed successfully");
}

void Mesh::drawRange(std::uint32_t indexOffset, std::uint32_t indexCount,
                     GLenum prim) const {
  if (!indexed_ || vao_ == 0) return;
  GL_CHECK(glBindVertexArray(vao_));
  GL_CHECK(glDrawElements(
      prim, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT,
      reinterpret_cast<void*>(static_cast<std::uintptr_t>(indexOffset) *
                              sizeof(std::uint32_t))));
  GL_CHECK(glBindVertexArray(0));
}

void Mesh::draw(GLenum prim) const {
  if (vao_ == 0) {
    LOG_ERROR("Mesh::draw - VAO is 0, mesh not uploaded!");
    return;
  }

  GL_CHECK(glBindVertexArray(vao_));
  if (indexed_) {
    GL_CHECK(glDrawElements(prim, indexCount_, GL_UNSIGNED_INT, nullptr));
  } else {
    GL_CHECK(glDrawArrays(prim, 0, vertexCount_));
  }
  GL_CHECK(glBindVertexArray(0));
}
}  // namespace utils
