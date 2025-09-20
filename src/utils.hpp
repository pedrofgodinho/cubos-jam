#include <glm/vec3.hpp>

const float BLOCK_SCALE = 5.0f;
const glm::vec3 CENTER_POS = glm::vec3(0.0f);

const glm::vec3 LEFT_CORNER = CENTER_POS - glm::vec3(BLOCK_SCALE * 5, BLOCK_SCALE * 5, 0);

glm::vec3 gridToWorld(int x, int y, int z);