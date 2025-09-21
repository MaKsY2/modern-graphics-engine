cmake_minimum_required(VERSION 3.16)

project(MODERN_GRAPHICS_ENGINE 
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics Window System)

add_executable(engine src/main.cpp)

target_link_libraries(engine PRIVATE SFML::Graphics SFML::Window SFML::System)

set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/engine
    DEPENDS engine
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running the engine..."
)
