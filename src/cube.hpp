#pragma once

#include <glm/vec3.hpp>

#include <cubos/engine/prelude.hpp>

struct Cube
{
    CUBOS_REFLECT;

    int trackingIndex = 0;
};

void cubePlugin(cubos::engine::Cubos& cubos);
