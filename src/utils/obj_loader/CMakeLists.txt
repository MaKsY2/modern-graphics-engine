add_library(obj_loader STATIC
    gltfLoaderTiny.cpp
)

target_include_directories(obj_loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(obj_loader PUBLIC
    utils
    glad
    tinygltf
    glm::glm 
    glfw
)