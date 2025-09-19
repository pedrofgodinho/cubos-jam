#pragma once

#include <cubos/engine/prelude.hpp>

#include <array>
#include <vector>

struct Game
{
    CUBOS_REFLECT;

    float tickAccumulator = 0.0F;
    float tickPeriod = 0.8F;

    int ticksToLock = 3;
    int tickLockAccumulator = 0;

    std::vector<std::vector<std::vector<int>>> board = std::vector<std::vector<std::vector<int>>>(
        10, std::vector<std::vector<int>>(20, std::vector<int>(10, 0)));

    // Sparse floating piece
    int floatingPieceColor = 0;

    // Coordinates of each block. [block

    std::vector<int> blockX{};
    std::vector<int> blockY{};
    std::vector<int> blockZ{};
};

void gameLogicPlugin(cubos::engine::Cubos& cubos);
