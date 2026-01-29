#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "render_object.hpp"

namespace primitives
{
    class Sphere : public utils::RenderObject
    {
    public:
        Sphere(float, uint32_t, uint32_t, const std::shared_ptr<Shader> &);

    private:
        static std::shared_ptr<utils::Mesh> getCachedMesh(uint32_t, uint32_t);
    };

} // namespace primitives

#endif