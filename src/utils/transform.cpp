#include "transform.hpp"

namespace utils
{
    const glm::mat4 &Transform::buildMatrix() const
    {
        if (dirty)
        {
            cached = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
            dirty = false;
        }
        return cached;
    }
} // namespace utils
