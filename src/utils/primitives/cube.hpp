#ifndef CUBE_HPP
#define CUBE_HPP

#include "render_object.hpp"

namespace primitives
{
    class Cube : public utils::RenderObject
    {
    public:
        Cube(const glm::vec3 &, const std::shared_ptr<Shader>&);
    };

} // namespace primitives

#endif