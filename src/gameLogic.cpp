#include "gameLogic.hpp"

#include <cubos/core/ecs/reflection.hpp>
#include <cubos/core/reflection/external/glm.hpp>
#include <cubos/core/reflection/external/primitives.hpp>
#include <cubos/core/reflection/external/string.hpp>
#include <cubos/core/reflection/external/vector.hpp>

#include <cubos/core/tel/logging.hpp>
#include <cubos/engine/assets/plugin.hpp>
#include <cubos/engine/transform/plugin.hpp>

using namespace cubos::engine;

CUBOS_REFLECT_IMPL(Game)
{
    return cubos::core::ecs::TypeBuilder<Game>("Game")
        .withField("tickAccumulator", &Game::tickAccumulator)
        .withField("tickPeriod", &Game::tickPeriod)
        .withField("board", &Game::board)
        .withField("boardGen", &Game::boardGen)
        .withField("floatingPieceColor", &Game::floatingPieceColor)
        .withField("blockX", &Game::blockX)
        .withField("blockY", &Game::blockY)
        .withField("blockZ", &Game::blockZ)
        .withField("score", &Game::score)
        .build();
}

void spawnBlock(Game& game)
{
    CUBOS_INFO("Spawning new piece");

    game.floatingPieceColor = 1;

    // Simple 2x2 square piece
    game.blockX = {0, 1, 0, 1};
    game.blockY = {19, 19, 19, 19};
    game.blockZ = {0, 0, 1, 1};
}

// Returns true if the block could move down, false if it hit something
bool moveBlockDown(Game& game)
{
    int numBlocks = game.blockX.size();
    for (int i = 0; i < numBlocks; i++)
    {
        int x = game.blockX[i];
        int y = game.blockY[i];
        int z = game.blockZ[i];

        // Check if we can move down
        if (y == 0 || game.board[x][y - 1][z] != 0)
        {
            return false;
        }
    }
    CUBOS_INFO("Moving block down to y = {}", game.blockY[0] - 1);
    // Move down
    for (int i = 0; i < numBlocks; i++)
    {
        game.blockY[i]--;
    }
    return true;
}

bool isPositionValid(const Game& game, int x, int y, int z)
{
    if (x < 0 || x >= 10 || y < 0 || y >= 20 || z < 0 || z >= 10)
    {
        return false;
    }
    if (game.board[x][y][z] != 0)
    {
        return false;
    }
    return true;
}

bool moveBlock(Game& game, Direction dir)
{
    int dx = 0;
    int dz = 0;
    switch (dir)
    {
    case NORTH:
        dx = 1;
        break;
    case EAST:
        dz = 1;
        break;
    case SOUTH:
        dx = -1;
        break;
    case WEST:
        dz = -1;
        break;
    }

    int numBlocks = game.blockX.size();
    // Check if we can move
    for (int i = 0; i < numBlocks; i++)
    {
        int x = game.blockX[i];
        int y = game.blockY[i];
        int z = game.blockZ[i];

        if (!isPositionValid(game, x + dx, y, z + dz))
        {
            return false;
        }
    }

    // Move
    for (int i = 0; i < numBlocks; i++)
    {
        game.blockX[i] += dx;
        game.blockZ[i] += dz;
    }
    return true;
}

void tryToClear(Game& game)
{
    CUBOS_INFO("Trying to clear lines...");
    // TODO
}

void lockFloatingBlock(Game& game)
{
    int numBlocks = game.blockX.size();
    for (int i = 0; i < numBlocks; i++)
    {
        int x = game.blockX[i];
        int y = game.blockY[i];
        int z = game.blockZ[i];

        game.board[x][y][z] = game.floatingPieceColor;
    }

    // Reset floating piece
    game.floatingPieceColor = 0;
    game.blockX.clear();
    game.blockY.clear();
    game.blockZ.clear();

    // Reset tick lock accumulator
    game.tickLockAccumulator = 0;

    tryToClear(game);

    game.boardGen++;
}

void gameLogicPlugin(Cubos& cubos)
{
    cubos.resource<Game>();

    cubos.system("game logic")
        .call([](Commands cmds, const DeltaTime& dt, Game& game) {
            // Accumulate time
            game.tickAccumulator += dt.value();
            if (game.tickAccumulator < game.tickPeriod)
            {
                return;
            }
            game.tickAccumulator -= game.tickPeriod;

            // Tick

            // Spawn block
            if (game.floatingPieceColor == 0)
            {
                spawnBlock(game);
            }

            if (!moveBlockDown(game))
            {
                CUBOS_INFO("Block cannot move down further.");

                if (game.tickLockAccumulator < game.ticksToLock)
                {
                    game.tickLockAccumulator++;
                    CUBOS_INFO("Tick lock accumulator: {}", game.tickLockAccumulator);
                } else
                {
                    CUBOS_INFO("Tick lock released, locking block in place.");
                    lockFloatingBlock(game);
                }
            } else
            {
                game.tickLockAccumulator = 0; // reset if we moved down, in case we adjust the block after landing
            }
        });
}


