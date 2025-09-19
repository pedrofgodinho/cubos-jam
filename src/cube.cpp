#include "cube.hpp"

#include "gameLogic.hpp"

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

void cubePlugin(Cubos& cubos)
{
    cubos.depends(assetsPlugin);
    cubos.depends(transformPlugin);
    cubos.depends(gameLogicPlugin);

    cubos.component<Cube>();

    cubos.system("track falling block")
        .call([](Commands cmds, const Game& game, Query<Entity, const Cube&, Position&> cubes) {
            for (auto [ent, cube, position] : cubes)
            {
                auto new_position = glm::vec3(
                    game.blockX[cube.trackingIndex]*5, game.blockY[cube.trackingIndex]*5, game.blockZ[cube.trackingIndex]*5
                );
                if (position.vec == new_position)
                {
                    continue;
                }
                position.vec = new_position;
                CUBOS_INFO("Updating cube {} to position {}, {}, {}", ent, position.vec.x, position.vec.y, position.vec.z);
                CUBOS_INFO("{}", ent);
            }
        });
}
