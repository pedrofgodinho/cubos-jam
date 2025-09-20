#include <glm/vec3.hpp>

const float CUBE_SCALE = 5.0f;
const glm::vec3 CENTER_POS = glm::vec3(0.0f);

const glm::vec3 LEFT_CORNER = CENTER_POS - glm::vec3(CUBE_SCALE * 5, CUBE_SCALE * 5, 0);

glm::vec3 gridToWorld(int x, int y, int z);