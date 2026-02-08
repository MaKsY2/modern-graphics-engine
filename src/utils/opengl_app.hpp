#ifndef OPENGL_APP_HPP
#define OPENGL_APP_HPP

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "shader.hpp"

namespace engine {
class OpenGLApp {
 private:
  static const uint32_t WIDTH = 1280;
  static const uint32_t HEIGHT = 800;
  GLFWwindow* window_;
  Shader* shader_;

  void init_window();
  void init_opengl();

 public:
  OpenGLApp();
  ~OpenGLApp();
};

}  // namespace engine

#endif