#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>

namespace utils
{
    class Object
    {
    public:
        void render(const glm::mat4 &viewProj);
    };
}

#endif