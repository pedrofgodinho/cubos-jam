#include "audioAudioPlugin.hpp"

#include <cubos/core/ecs/reflection.hpp>

#include <cubos/engine/assets/plugin.hpp>

#include <cubos/engine/audio/plugin.hpp>
#include <cubos/engine/prelude.hpp>
#include <cubos/engine/transform/position.hpp>
#include <cubos/engine/transform/rotation.hpp>
#include <cubos/engine/physics/components/velocity.hpp>

using namespace cubos::engine;

static const Asset<Audio> BackgroundMusic = AnyAsset("assets/audio/cubosmusic.wav");

void audioAudioPlugin(Cubos& cubos)
{
    cubos.depends(audioPlugin);
    cubos.depends(assetsPlugin);

    cubos.startupSystem("create an audio listener").after(audioStateInitTag).call([](Commands cmds) {
        cmds.create()
            .add(Position{{8.0F, 1.0F, -2.0F}})
            .add(Velocity{.vec = {1.0F, 1.0F, 1.0F}})
            .add(Rotation{})
            .add(AudioListener{true});
    });

    cubos.startupSystem("create a background audio stream").after(audioStateInitTag).after(assetsTag).call([](Commands cmds, Assets& assets) {
        cmds.create()
            .add(Position{{8.0F, 1.0F, -2.0F}})
            .add(Velocity{.vec = {1.0F, 1.0F, 1.0F}})
            .add(Rotation{})
            .add(AudioSource{ .looping = true, .gain = 0.75, .sound = BackgroundMusic })
            .add(AudioPlay{});
    });

    //cubos.observer("play sounds immediately")
    //    .onAdd<AudioSource>()
    //    .without<AudioPlay>()
    //    .call([](Commands cmds, Query<Entity> query) {
    //        for( auto [ ent ] : query) {
    //            cmds.add(ent, AudioPlay{});
    //        }
    //    });
}