#include "render_object.hpp"

#include <memory>

#include "shader.hpp"

namespace utils

{
RenderObject::RenderObject(const std::shared_ptr<Mesh>& mesh,
                           const std::shared_ptr<Shader>& shader)
    : mesh_(std::move(mesh)), shader_(std::move(shader)) {}

void RenderObject::draw(const glm::mat4& viewProj) const {
  glm::mat4 model = transform.buildMatrix();
  shader_->use();
  shader_->setMat4("uModel", model);
  shader_->setMat4("uViewProj", viewProj);
  shader_->setVec4("uColor", color);
  shader_->setVec3("uLightDirW", glm::normalize(glm::vec3(1, 1, 1)));
  mesh_->draw();
}
}  // namespace utils
