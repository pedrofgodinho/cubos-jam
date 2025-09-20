#include "utils.hpp"

#include <glm/vec3.hpp>

glm::vec3 gridToWorld(int x, int y, int z)
{
    return LEFT_CORNER + glm::vec3(float(x * BLOCK_SCALE), float(y * BLOCK_SCALE), float(z * BLOCK_SCALE));
}