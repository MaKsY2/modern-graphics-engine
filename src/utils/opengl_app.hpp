#ifndef OPENGL_APP_HPP
#define OPENGL_APP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.hpp"

namespace engine
{
    class OpenGLApp
    {
    private:
        static const size_t WIDTH = 1280;
        static const size_t HEIGHT = 800;
        GLFWwindow *window_;
        Shader *shader_;


        void init_window();
        void init_opengl();

    public:
        OpenGLApp();
        ~OpenGLApp();
    };

} // namespace engine

#endif