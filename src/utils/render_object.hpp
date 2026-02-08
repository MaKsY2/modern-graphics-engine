#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include <memory>

#include "mesh.hpp"
#include "shader.hpp"
#include "transform.hpp"

namespace utils {

class RenderObject {
 private:
  std::shared_ptr<Mesh> mesh_;
  std::shared_ptr<Shader> shader_;
  std::shared_ptr<ModelData> modelData_;

 public:
  Transform transform;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  RenderObject(const std::shared_ptr<Mesh>&, const std::shared_ptr<Shader>&);
  RenderObject(const std::shared_ptr<Mesh>& mesh,
               const std::shared_ptr<Shader>& shader,
               const std::shared_ptr<ModelData>& modelData);
  virtual ~RenderObject() = default;

  virtual void draw(const glm::mat4&) const;
};

}  // namespace utils

#endif