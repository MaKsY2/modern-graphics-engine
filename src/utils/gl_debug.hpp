#ifndef GL_DEBUG_HPP
#define GL_DEBUG_HPP

#include <glad/glad.h>
#include <iostream>
#include <string>

namespace utils {

inline void checkGLError(const char* file, int line, const char* call) {
  GLenum err;
  bool hasError = false;
  while ((err = glGetError()) != GL_NO_ERROR) {
    hasError = true;
    std::string error;
    switch (err) {
      case GL_INVALID_ENUM:
        error = "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        error = "GL_INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        error = "GL_INVALID_OPERATION";
        break;
      case GL_OUT_OF_MEMORY:
        error = "GL_OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        error = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      default:
        error = "UNKNOWN_ERROR";
        break;
    }
    std::cerr << "[GL ERROR] " << error << " at " << file << ":" << line
              << " in call: " << call << std::endl;
  }
  if (hasError) {
    std::cerr << "[GL ERROR] Stack trace available with debugger" << std::endl;
  }
}

#ifdef NDEBUG
#define GL_CHECK(call) call
#else
#define GL_CHECK(call)                                  \
  do {                                                  \
    call;                                               \
    utils::checkGLError(__FILE__, __LINE__, #call);    \
  } while (0)
#endif

#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define LOG_WARN(msg) std::cerr << "[WARN] " << msg << std::endl

}  // namespace utils

#endif