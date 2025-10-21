cmake_minimum_required(VERSION 3.16)
project(FABRIK_OpenGL CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenGL
find_package(OpenGL REQUIRED)
# GLFW (через brew install glfw)
find_package(glfw3 REQUIRED)
# GLEW (через brew install glew)
find_package(GLEW REQUIRED)

# GLM — header-only (через brew install glm). У большинства лежит в /opt/homebrew/include
# Если у тебя другой путь — поменяй строку ниже или убери, если GLM уже находится компилятором
include_directories(/opt/homebrew/include)

add_executable(fabrik_opengl
  main.cpp
)

target_link_libraries(fabrik_opengl
  OpenGL::GL
  glfw
  GLEW::GLEW
)

if(APPLE)
  target_compile_definitions(fabrik_opengl PRIVATE GL_SILENCE_DEPRECATION)
  # На macOS нужен Core Profile 3.3 — укажем в коде glfwWindowHint
endif()
