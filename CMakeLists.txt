cmake_minimum_required(VERSION 3.16)

project(MODERN_GRAPHICS_ENGINE
    VERSION 1.0.0
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(src/libs/glad)
add_subdirectory(src/libs/tinygltf-2.9.7)
add_subdirectory(src/utils)

add_executable(main_app
    src/main.cpp
)

target_include_directories(main_app PRIVATE
    src
)

target_link_libraries(main_app PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    tinygltf
    glad
    obj_loader
    primitives
    utils
)

set_target_properties(main_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/shaders
        ${CMAKE_BINARY_DIR}/bin/shaders
    COMMENT "Copying shader files..."
)

add_dependencies(main_app copy_shaders)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/main_app
    DEPENDS main_app
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running main application..."
)

message(STATUS "=== Modern Graphics Engine Configuration ===")
message(STATUS "OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "GLFW3: Found")
message(STATUS "GLM: Found")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "==========================================")