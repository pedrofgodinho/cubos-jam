#include "cube.hpp"

#include "gameLogic.hpp"
#include "utils.hpp"

#include <cubos/core/ecs/reflection.hpp>
#include <cubos/core/reflection/external/glm.hpp>
#include <cubos/core/reflection/external/primitives.hpp>
#include <cubos/core/reflection/external/string.hpp>

#include <cubos/engine/assets/plugin.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/utils/free_camera/controller.hpp>

using namespace cubos::engine;

CUBOS_REFLECT_IMPL(Cube)
{
    return cubos::core::ecs::TypeBuilder<Cube>("Obstacle")
        .withField("trackingIndex", &Cube::trackingIndex)
        .build();
}

CUBOS_REFLECT_IMPL(StationaryCube)
{
    return cubos::core::ecs::TypeBuilder<StationaryCube>("StationaryCube")
        .withField("gen", &StationaryCube::gen)
        .build();
}

void cubePlugin(Cubos& cubos)
{
    cubos.depends(assetsPlugin);
    cubos.depends(transformPlugin);
    cubos.depends(gameLogicPlugin);

    cubos.component<Cube>();
    cubos.component<StationaryCube>();

    cubos.system("track falling block")
        .call([](Commands cmds, const Game& game, Query<Entity, const Cube&, Position&> cubes) {
            for (auto [ent, cube, position] : cubes)
            {
                auto new_position = gridToWorld(
                    game.blockX[cube.trackingIndex],
                    game.blockY[cube.trackingIndex],
                    game.blockZ[cube.trackingIndex]
                );
                if (position.vec == new_position)
                {
                    continue;
                }
                position.vec = new_position;
                CUBOS_INFO("Updating cube {} to position {}, {}, {}", ent, position.vec.x, position.vec.y, position.vec.z);
            }
        });
}
