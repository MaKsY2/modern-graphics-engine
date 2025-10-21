cmake_minimum_required(VERSION 3.16)

project(MODERN_GRAPHICS_ENGINE
    VERSION 1.0.0
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics Window System)

# Добавляем GLAD как статическую библиотеку
add_library(glad STATIC src/production/glad/src/glad.c)
target_include_directories(glad PUBLIC src/production/glad/include)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

add_executable(engine_testing src/testing/main.cpp)
add_executable(engine_production src/production/main.cpp src/production/utils/shader.hpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Добавляем include директории для GLAD
target_include_directories(engine_production PRIVATE src/production/glad/include)

target_link_libraries(engine_testing PRIVATE
    OpenGL::GL
    glfw
    glm::glm)
target_link_libraries(engine_production PRIVATE
    OpenGL::GL
    glfw
    glad
    glm::glm)

set_target_properties(engine_testing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin_testing
)

set_target_properties(engine_production PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/engine_production
    DEPENDS engine_production
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running the engine..."
)

add_custom_target(run-testing
    COMMAND ${CMAKE_BINARY_DIR}/bin_testing/engine_testing
    DEPENDS engine_testing
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "[TESTING] Running the engine..."
)
