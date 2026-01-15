add_library(utils STATIC
    shader.cpp
    opengl_app.cpp
)

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(utils PUBLIC
    glad
    glm::glm
    glfw
)
