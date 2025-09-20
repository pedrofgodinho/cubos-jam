#include <cubos/engine/assets/plugin.hpp>
#include <cubos/engine/collisions/colliding_with.hpp>
#include <cubos/engine/defaults/plugin.hpp>
#include <cubos/engine/input/plugin.hpp>
#include <cubos/engine/render/lights/environment.hpp>
#include <cubos/engine/render/voxels/palette.hpp>
#include <cubos/engine/scene/plugin.hpp>
#include <cubos/engine/settings/plugin.hpp>
#include <cubos/engine/tools/plugin.hpp>
#include <cubos/engine/utils/free_camera/plugin.hpp>
#include <cubos/engine/voxels/plugin.hpp>

#include "cube.hpp"
#include "gameLogic.hpp"
#include "utils.hpp"

#include <cubos/engine/transform/position.hpp>

using namespace cubos::engine;

static const Asset<Scene> SceneAsset = AnyAsset("/assets/scenes/main.cubos");
static const Asset<Scene> CubeAsset = AnyAsset("/assets/scenes/cube.cubos");
static const Asset<VoxelPalette> PaletteAsset = AnyAsset("/assets/main.pal");
static const Asset<InputBindings> InputBindingsAsset = AnyAsset("/assets/input.bind");

int main(int argc, char** argv)
{
    Cubos cubos{argc, argv};

    cubos.plugin(defaultsPlugin);
    cubos.plugin(freeCameraPlugin);
    cubos.plugin(toolsPlugin);
    cubos.plugin(gameLogicPlugin);
    cubos.plugin(cubePlugin);

    cubos.startupSystem("configure settings").before(settingsTag).call([](Settings& settings) {
        settings.setString("assets.app.osPath", APP_ASSETS_PATH);
        settings.setString("assets.builtin.osPath", BUILTIN_ASSETS_PATH);
    });

    cubos.startupSystem("set the palette, environment, input bindings and spawn the scene")
        .tagged(assetsTag)
        .call([](Commands commands, const Assets& assets, RenderPalette& palette, Input& input,
                 RenderEnvironment& environment) {
            palette.asset = PaletteAsset;
            environment.ambient = {0.1F, 0.1F, 0.1F};
            environment.skyGradient[0] = {0.2F, 0.4F, 0.8F};
            environment.skyGradient[1] = {0.6F, 0.6F, 0.8F};
            input.bind(*assets.read(InputBindingsAsset));
            commands.spawn(assets.read(SceneAsset)->blueprint()).named("main");
        });

    cubos.system("restart the game on input")
        .call([](Commands cmds, const Assets& assets, const Input& input, Query<Entity> all) {
            if (input.justPressed("restart"))
            {
                for (auto [ent] : all)
                {
                    cmds.destroy(ent);
                }

                cmds.spawn(assets.read(SceneAsset)->blueprint()).named("main");
            }
        });

    cubos.system("spawn cubes for the falling block")
        .call([](Commands cmds, const Assets& assets, const Game& game, Query<const Cube&> existingCubes) {
            int numBlocks = game.blockX.size();
            int existingCubesCount = 0;
            for (auto [cube] : existingCubes)
            {
                (void)cube;
                existingCubesCount++;
            }

            for (int i = existingCubesCount; i < numBlocks; i++)
            {
                CUBOS_INFO("Creating cube for block index {}", i);
                cmds.spawn(*assets.read(CubeAsset)).named("cube").add(Cube{i});
            }
        });

    cubos.system("track existing cubes")
    .call([](Commands cmds, const Assets& assets, const Game& game, Query<Entity, const StationaryCube&> cubes) {
        bool needsUpdate = cubes.count() == 0;

        int count = 0;
        for (auto [ent, cube] : cubes)
        {

            if (cube.gen < game.boardGen)
            {
                cmds.destroy(ent);
                needsUpdate = true;
                count++;
            } else
            {
                break;
            }
        }

        if (needsUpdate && game.boardGen > 0)
        {
            CUBOS_INFO("Deleted {} old cubes for gen {}", count, game.boardGen);
            CUBOS_INFO("Updating board to gen {}", game.boardGen);
            for (int x = 0; x < 10; x++)
            {
                for (int y = 0; y < 20; y++)
                {
                    for (int z = 0; z < 10; z++)
                    {
                        if (game.board[x][y][z] != 0)
                        {
                            Position pos;
                            pos.vec = gridToWorld(x, y, z);
                            cmds.spawn(*assets.read(CubeAsset))
                                    .named("cube")
                                    .add(pos)
                                    .add(StationaryCube{game.boardGen});
                        }
                    }
                }
            }
        }
    });
    /*

    cubos.system("detect player vs obstacle collisions")
        .call([](Query<const Player&, const CollidingWith&, const Obstacle&> collisions) {
            for (auto [player, collidingWith, obstacle] : collisions)
            {
                CUBOS_INFO("Player collided with an obstacle!");
                (void)player; // here to shut up 'unused variable warning', you can remove it
            }
        });
        */

    cubos.run();
}
