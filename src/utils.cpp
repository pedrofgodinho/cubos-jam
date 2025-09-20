#include "utils.hpp"

#include <glm/vec3.hpp>

glm::vec3 gridToWorld(int x, int y, int z)
{
    return LEFT_CORNER + glm::vec3(float(x * CUBE_SCALE), float(y * CUBE_SCALE), float(z * CUBE_SCALE));
}