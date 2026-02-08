#ifndef GLTF_LOADER_TINY_HPP
#define GLTF_LOADER_TINY_HPP

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "../mesh.hpp"

namespace loader {

utils::ModelData LoadGLB_ToCPU(const std::string& path);

void DestroyModelTextures(utils::ModelData& m);

}  // namespace loader
#endif