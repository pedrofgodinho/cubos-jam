#include "camera.hpp"

#include "gameLogic.hpp"
#include "utils.hpp"

#include <cubos/core/ecs/reflection.hpp>
#include <cubos/engine/assets/plugin.hpp>
#include <cubos/engine/defaults/plugin.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/render/camera/perspective.hpp>
#include <cubos/engine/input/input.hpp>
#include <cubos/engine/input/plugin.hpp>

namespace cubos::engine
{
    class Input;
    struct PerspectiveCamera;
}
using namespace cubos::engine;

void cameraPlugin(Cubos& cubos)
{
    cubos.depends(inputPlugin);
    cubos.depends(defaultsPlugin);
    cubos.depends(gameLogicPlugin);

    // Why am I making this system in the camera plugin? Who knows
    cubos.system("move blocks")
        .call([](const Game& game, const DeltaTime& dt, Query<Position&> blocks) {
            // TODO
        });

    cubos.system("rotate camera")
        .call([](Commands cmds, const Input& input, Query<PerspectiveCamera&, Position&, Rotation&> camera) {
            // Get both horizontal and vertical mouse movement.
            glm::vec2 mouseDelta = input.mouseDelta();

            // If the mouse hasn't moved, there's nothing to do.
            if (glm::length(mouseDelta) < 0.001f)
            {
                return;
            }

            for (auto [cam, pos, rot] : camera)
            {
                // --- Constants ---
                const glm::vec3 CENTER_POS = {0.0F, 0.0F, 0.0F};
                const glm::vec3 LOOK_AT_OFFSET = {0.0F, 35.0F, 0.0F};

                // --- Sensitivity Modifiers ---
                // Separate sensitivity for horizontal (X) and vertical (Y) camera movement.
                const float SENSITIVITY_X = 0.003f;
                const float SENSITIVITY_Y = 0.003f;

                // --- Vertical Angle Limits ---
                const float MIN_PITCH_DEGREES = 5.0f;
                const float MAX_PITCH_DEGREES = 89.0f;
                const float MIN_PITCH_RADIANS = glm::radians(MIN_PITCH_DEGREES);
                const float MAX_PITCH_RADIANS = glm::radians(MAX_PITCH_DEGREES);

                // Get the current offset vector from the center to the camera.
                glm::vec3 offset = pos.vec - CENTER_POS;

                // --- 1. Horizontal Rotation (Yaw) ---
                // This now uses SENSITIVITY_X.
                float yawAngle = -mouseDelta.x * SENSITIVITY_X;
                glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                offset = glm::vec3(yawRotation * glm::vec4(offset, 1.0f));

                // --- 2. Vertical Rotation (Pitch) with Clamping ---
                // This now uses SENSITIVITY_Y.
                float pitchAngle = -mouseDelta.y * SENSITIVITY_Y;

                // Calculate the current vertical angle of the camera.
                float currentPitch = asinf(offset.y / glm::length(offset));

                // Clamp the desired new angle to stay within our min/max limits.
                float newPitch = glm::clamp(currentPitch + pitchAngle, MIN_PITCH_RADIANS, MAX_PITCH_RADIANS);

                // Calculate the actual angle we need to rotate by to reach the clamped angle.
                float actualPitchAngle = newPitch - currentPitch;

                if (abs(actualPitchAngle) > 0.001f)
                {
                    glm::vec3 pitchAxis = glm::normalize(glm::cross(offset, glm::vec3(0.0f, 1.0f, 0.0f)));
                    glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), actualPitchAngle, pitchAxis);
                    offset = glm::vec3(pitchRotation * glm::vec4(offset, 1.0f));
                }

                // --- 3. Final Update ---
                pos.vec = CENTER_POS + offset;
                rot = Rotation::lookingAt(glm::normalize(CENTER_POS - pos.vec + LOOK_AT_OFFSET));
            }
    });


}
