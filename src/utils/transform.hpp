#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace utils {

struct Transform {
  glm::vec3 position{0, 0, 0};
  glm::quat rotation{1, 0, 0, 0};
  glm::vec3 scale{1, 1, 1};

  mutable bool dirty = true;
  mutable glm::mat4 cached{1.0f};

  const glm::mat4& buildMatrix() const;
};

}  // namespace utils

#endif