# Cubos Jam Template Repository

This repository holds a template game, **Cubos Surfers**, that can be used as a base for creating games in the **Cubos** engine. The template is based on one of the engine's samples, which can be found [here](https://github.com/GameDevTecnico/cubos/tree/main/engine/samples/games/cubosurfers).

## Building

To build the template you need to first separately install the **Cubos Engine** using *CMake*.
Then, when building this repository, you might need to help *CMake* find the installation directory of **Cubos**.

We recommend building both the engine and the template in Release - when developing, Release with Debug Information is usually enough to debug any errors that show up.

## Distributing

When compiling the game during development, paths to assets directories are hardcoded into the produced binaries.
This means that the produced binaries are not portable and won't work on other machines.
To create a portable installation of the game, enable the *CMake* option `DISTRIBUTE`, and then install the project through *CMake*.
