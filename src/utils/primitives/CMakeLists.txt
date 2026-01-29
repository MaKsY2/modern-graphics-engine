add_library(primitives STATIC
    cube.cpp
    sphere.cpp
)

target_include_directories(primitives PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(primitives PUBLIC
    utils
    glad
    glm::glm
    glfw
)
