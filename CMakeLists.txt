# CMakeLists.txt
# Cubos template project build configuration

cmake_minimum_required(VERSION 3.25.0)
project(cubos-template VERSION 0.1.0)

find_package(cubos REQUIRED)

option(DISTRIBUTE "Build the game for distribution" OFF)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize-recover=address")

add_executable(game
    src/main.cpp
    src/obstacle.cpp
    src/player.cpp
    src/spawner.cpp
)

target_link_libraries(game cubos::engine)
target_compile_features(game PRIVATE cxx_std_20)

# Enable all warnings and treat them as errors
target_compile_options(game PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /Zc:preprocessor> # Enable preprocessor conformance mode - required for __VA_ARGS__ to work correctly
)

if(EMSCRIPTEN)
    set_target_properties(game PROPERTIES SUFFIX ".html")
endif()

if(EMSCRIPTEN)
    target_link_options(game PRIVATE
        "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
        "SHELL:--embed-file ${CUBOS_ENGINE_ASSETS_PATH}@/builtin"
        "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/emscripten.html"
    )
    target_compile_definitions(game PRIVATE
        APP_ASSETS_PATH="/assets"
        BUILTIN_ASSETS_PATH="/builtin"
    )
else()
    if(DISTRIBUTE)
        target_compile_definitions(game PRIVATE
            APP_ASSETS_PATH="assets"
            BUILTIN_ASSETS_PATH="builtin"
        )
    else()
        target_compile_definitions(game PRIVATE
            APP_ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets"
            BUILTIN_ASSETS_PATH="${CUBOS_ENGINE_ASSETS_PATH}"
        )
    endif()
endif()

# ------------------------ Configure game installation ------------------------

if(DISTRIBUTE)
    if(EMSCRIPTEN)
        install(FILES $<TARGET_FILE_DIR:game>/game.html DESTINATION game)
        install(FILES $<TARGET_FILE_DIR:game>/game.wasm DESTINATION game)
        install(FILES $<TARGET_FILE_DIR:game>/game.js DESTINATION game)
    else()
        install(TARGETS game EXPORT game RUNTIME DESTINATION game)
    endif()
    install(DIRECTORY assets/ DESTINATION game/assets)
    install(DIRECTORY ${CUBOS_ENGINE_ASSETS_PATH}/ DESTINATION game/builtin)
endif()
