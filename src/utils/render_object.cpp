#include "render_object.hpp"

#include <iostream>
#include <memory>

#include "shader.hpp"

namespace utils

{
RenderObject::RenderObject(const std::shared_ptr<Mesh>& mesh,
                           const std::shared_ptr<Shader>& shader)
    : mesh_(std::move(mesh)), shader_(std::move(shader)) {}

RenderObject::RenderObject(const std::shared_ptr<Mesh>& mesh,
                           const std::shared_ptr<Shader>& shader,
                           const std::shared_ptr<ModelData>& modelData)
    : mesh_(std::move(mesh)),
      shader_(std::move(shader)),
      modelData_(std::move(modelData)) {}

void RenderObject::draw(const glm::mat4& viewProj) const {
  glm::mat4 model = transform.buildMatrix();
  shader_->use();
  shader_->setMat4("uModel", model);
  shader_->setMat4("uViewProj", viewProj);
  shader_->setVec3("uLightDirW", glm::normalize(glm::vec3(1, 1, 1)));

  if (!modelData_ || modelData_->submeshes.empty()) {
    shader_->setVec4("uBaseColorFactor", color);
    shader_->setBool("uHasBaseColorTex", false);
    mesh_->draw();
    return;
  }

  for (const auto& submesh : modelData_->submeshes) {
    glm::vec4 factor = glm::vec4(1, 1, 1, 1);
    bool hasTex = false;
    GLuint tex = 0;

    if (submesh.materialIndex >= 0 &&
        submesh.materialIndex < modelData_->materials.size()) {
      const auto& mat = modelData_->materials[submesh.materialIndex];
      factor = mat.baseColorFactor;
      hasTex = mat.hasBaseColorTex;
      tex = mat.baseColorTex;
    }

    shader_->setVec4("uBaseColorFactor", factor);
    shader_->setBool("uHasBaseColorTex", hasTex);

    if (hasTex && tex != 0) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);
      shader_->setInt("uBaseColorTex", 0);
    } else {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    mesh_->drawRange(submesh.indexOffset, submesh.indexCount);
  }
}
}  // namespace utils
