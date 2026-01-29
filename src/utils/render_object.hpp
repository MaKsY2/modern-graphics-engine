#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include "transform.hpp"
#include "shader.hpp"
#include "mesh.hpp"

#include <memory>

namespace utils
{

    class RenderObject
    {
    private:
        std::shared_ptr<Mesh> mesh_;
        std::shared_ptr<Shader> shader_;

    public:
        Transform transform;
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

        RenderObject(const std::shared_ptr<Mesh> &, const std::shared_ptr<Shader> &);
        virtual ~RenderObject() = default;

        virtual void draw(const glm::mat4 &) const;
    };

} // namespace utils

#endif