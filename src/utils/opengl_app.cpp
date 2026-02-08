#include "opengl_app.hpp"

#include "logger.hpp"

namespace engine {
namespace {}  // namespace
void OpenGLApp::init_window() {
  LOG("Initializing GLFW window...");
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  window_ = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Cube", nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);
  glfwSetWindowUserPointer(window_, this);
  LOG("Window created successfully");
}
void OpenGLApp::init_opengl() {}

OpenGLApp::OpenGLApp() { init_window(); }

OpenGLApp::~OpenGLApp() {}
}  // namespace engine
