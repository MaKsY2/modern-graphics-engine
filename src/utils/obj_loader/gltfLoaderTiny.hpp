#ifndef GLTF_LOADER_TINY_HPP
#define GLTF_LOADER_TINY_HPP

#include "../mesh.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace loader
{
    struct LoadedVertex
    {
        glm::vec3 pos{};
        glm::vec3 normal{};
    };

    struct LoadedMeshPU
    {
        std::vector<utils::VertexPU> vertices;
        std::vector<std::uint32_t> indices;
    };
    // что-то типо передаем в цпу vertices и indices
    // Добавить std::runtime_error
    LoadedMeshPU LoadGLB_ToCPU_PU(const std::string &path, bool flipV);

}
#endif