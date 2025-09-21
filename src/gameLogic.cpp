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

void spawnBlock(Game& game)
{
    CUBOS_INFO("Spawning new piece");

    // Define the shapes of the pieces using relative coordinates from a pivot point.
    // Each inner vector is {dx, dy, dz}. dy is always 0 for a flat spawn.
    const std::vector<std::vector<std::vector<int>>> pieceShapes = {
        // I-piece (Line)
        {{0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {0, 0, 3}},
        // O-piece (Square)
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 1}, {1, 0, 1}},
        // T-piece
        {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {1, 0, -1}},
        // L-piece
        {{0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {1, 0, 2}},
        // J-piece (Reversed L)
        {{1, 0, 0}, {1, 0, 1}, {1, 0, 2}, {0, 0, 2}},
        // S-piece
        {{1, 0, 0}, {2, 0, 0}, {0, 0, 1}, {1, 0, 1}},
        // Z-piece (Reversed S)
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {2, 0, 1}}};

    // Pick a random piece and color.
    int pieceType = rand() % pieceShapes.size();
    game.floatingPieceColor = (rand() % 5) + 1; // Colors 1-5

    const auto& shape = pieceShapes[pieceType];

    // Clear the old floating piece data.
    game.blockX.clear();
    game.blockY.clear();
    game.blockZ.clear();

    // Set spawn position at top-center.
    int startX = 4;
    int startY = 18;
    int startZ = 4;

    // Create the new piece and check for game over.
    for (const auto& block : shape)
    {
        int x = startX + block[0];
        int y = startY + block[1];
        int z = startZ + block[2];

        if (!isPositionValid(game, x, y, z))
        {
            CUBOS_CRITICAL("GAME OVER: Cannot spawn new piece in an occupied space.");
            // Here you could implement a proper game over state.
            // For now, we just stop spawning.
            game.floatingPieceColor = 0; // Prevent further movement
            return;
        }

        game.blockX.push_back(x);
        game.blockY.push_back(y);
        game.blockZ.push_back(z);
    }
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

bool tryToClear(Game& game)
{
    // --- Scan for and clear one complete X-line ---
    for (int y = 0; y < 20; ++y)
    {
        for (int z = 0; z < 10; ++z)
        {
            bool lineIsFull = true;
            for (int x = 0; x < 10; ++x)
            {
                if (game.board[x][y][z] == 0)
                {
                    lineIsFull = false;
                    break;
                }
            }

            if (lineIsFull)
            {
                CUBOS_INFO("Clearing X-line at y={}, z={}", y, z);
                // Shift the slice above this line down.
                for (int shiftY = y; shiftY < 19; ++shiftY)
                {
                    for (int x = 0; x < 10; ++x)
                    {
                        game.board[x][shiftY][z] = game.board[x][shiftY + 1][z];
                    }
                }
                // Clear the top-most line of the slice.
                for (int x = 0; x < 10; ++x)
                {
                    game.board[x][19][z] = 0;
                }

                game.score += 10; // Add points for one line
                game.boardGen++;
                return true; // A line was cleared, exit to process next tick.
            }
        }
    }

    // --- Scan for and clear one complete Z-line ---
    for (int y = 0; y < 20; ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            bool lineIsFull = true;
            for (int z = 0; z < 10; ++z)
            {
                if (game.board[x][y][z] == 0)
                {
                    lineIsFull = false;
                    break;
                }
            }

            if (lineIsFull)
            {
                CUBOS_INFO("Clearing Z-line at y={}, x={}", y, x);
                // Shift the slice above this line down.
                for (int shiftY = y; shiftY < 19; ++shiftY)
                {
                    for (int z = 0; z < 10; ++z)
                    {
                        game.board[x][shiftY][z] = game.board[x][shiftY + 1][z];
                    }
                }
                // Clear the top-most line of the slice.
                for (int z = 0; z < 10; ++z)
                {
                    game.board[x][19][z] = 0;
                }

                game.score += 10; // Add points for one line
                game.boardGen++;
                return true; // A line was cleared, exit to process next tick.
            }
        }
    }

    // No complete lines were found in the entire board.
    return false;
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



            // If there is no block,
            if (game.floatingPieceColor == 0)
            {
                // If there is no block, a block was just locked in place, so try to clear lines
                if (!tryToClear(game))
                {
                    // If nothing to clear, spawn a new block
                    spawnBlock(game);
                    // This makes it so we do each step of clearing on a separate tick, so it "animates" the clearing
                } else
                {
                    // If we cleared something, don't do the rest of the tick logic yet
                    return;
                }
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


