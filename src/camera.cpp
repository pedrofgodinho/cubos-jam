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

// Helper function to find the dominant cardinal direction for a given vector. (Thanks gemini)
Direction getDominantDirection(glm::vec3 direction)
{
    // Define the four cardinal direction vectors based on your spec (North is +X, East is +Z).
    const glm::vec3 VEC_NORTH = {1.0f, 0.0f, 0.0f};
    const glm::vec3 VEC_EAST = {0.0f, 0.0f, 1.0f};
    const glm::vec3 VEC_SOUTH = {-1.0f, 0.0f, 0.0f};
    const glm::vec3 VEC_WEST = {0.0f, 0.0f, -1.0f};

    // Store directions and their dot products with the input direction.
    std::vector<std::pair<float, Direction>> products;
    products.push_back({glm::dot(direction, VEC_NORTH), NORTH});
    products.push_back({glm::dot(direction, VEC_EAST), EAST});
    products.push_back({glm::dot(direction, VEC_SOUTH), SOUTH});
    products.push_back({glm::dot(direction, VEC_WEST), WEST});

    // Find the direction with the highest dot product (i.e., the most similar direction).
    auto maxElement = std::max_element(products.begin(), products.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

    return maxElement->second;
}

void cameraPlugin(Cubos& cubos)
{
    cubos.depends(inputPlugin);
    cubos.depends(defaultsPlugin);
    cubos.depends(gameLogicPlugin);

    // Why am I making this system in the camera plugin? Who knows
    cubos.system("move blocks")
        .call([](const Game& game, const Input& input, Query<const Position&> cameraQuery) {
            // There should only be one camera, so we get the first result.
            if (cameraQuery.empty())
            {
                return; // No camera found.
            }
            auto [cameraPos] = *cameraQuery.begin();

            // --- Calculate Camera-Relative Direction Vectors ---
            const glm::vec3 CENTER_POS = {0.0f, 0.0f, 0.0f};

            // 1. Get the camera's "look" direction and project it onto the XZ plane.
            glm::vec3 forward = CENTER_POS - cameraPos.vec;
            forward.y = 0.0f; // Ignore vertical component for planar movement.
            forward = glm::normalize(forward);

            // 2. Get the camera's "right" direction using the cross product.
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward));

            // --- Handle Input ---
            if (input.justPressed("up"))
            {
                moveBlock(const_cast<Game&>(game), getDominantDirection(forward));
            }
            if (input.justPressed("down"))
            {
                moveBlock(const_cast<Game&>(game), getDominantDirection(-forward)); // -forward is towards the camera
            }
            if (input.justPressed("right"))
            {
                moveBlock(const_cast<Game&>(game), getDominantDirection(-right));
            }
            if (input.justPressed("left"))
            {
                moveBlock(const_cast<Game&>(game), getDominantDirection(right));
            }
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
