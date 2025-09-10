# cubos_surfers/CMakeLists.txt
# Cubos Surfers build configuration

add_executable(cubos_surfers
    src/main.cpp
    src/obstacle.cpp
    src/player.cpp
    src/spawner.cpp
)

target_link_libraries(cubos_surfers cubos::engine)
target_compile_features(cubos_surfers PRIVATE cxx_std_20)
common_demo_options(cubos_surfers)

if(EMSCRIPTEN)
    target_link_options(cubos_surfers PRIVATE
        "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
        "SHELL:--embed-file ${CUBOS_ENGINE_ASSETS_PATH}@/builtin"
        "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/emscripten.html"
    )
    target_compile_definitions(cubos_surfers PRIVATE
        APP_ASSETS_PATH="/assets"
        BUILTIN_ASSETS_PATH="/builtin"
    )
else()
    if(DISTRIBUTE)
        target_compile_definitions(cubos_surfers PRIVATE
            APP_ASSETS_PATH="assets"
            BUILTIN_ASSETS_PATH="builtin"
        )
    else()
        target_compile_definitions(cubos_surfers PRIVATE
            APP_ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets"
            BUILTIN_ASSETS_PATH="${CUBOS_ENGINE_ASSETS_PATH}"
        )
    endif()
endif()

# ------------------------ Configure game installation ------------------------

if(DISTRIBUTE)
    if(EMSCRIPTEN)
        install(FILES $<TARGET_FILE_DIR:cubos_surfers>/cubos_surfers.html DESTINATION cubos_surfers)
        install(FILES $<TARGET_FILE_DIR:cubos_surfers>/cubos_surfers.wasm DESTINATION cubos_surfers)
        install(FILES $<TARGET_FILE_DIR:cubos_surfers>/cubos_surfers.js DESTINATION cubos_surfers)
    else()
        install(TARGETS cubos_surfers EXPORT cubos_surfers RUNTIME DESTINATION cubos_surfers)
    endif()
    install(DIRECTORY assets/ DESTINATION cubos_surfers/assets)
    install(DIRECTORY ${CUBOS_ENGINE_ASSETS_PATH}/ DESTINATION cubos_surfers/builtin)
endif()
