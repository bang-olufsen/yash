project(external)

include(ExternalProject)
cmake_minimum_required(VERSION 3.1)

ExternalProject_Add(
    turtle
    GIT_REPOSITORY https://github.com/mat007/turtle
    GIT_TAG v1.3.2
    SOURCE_DIR ../src/external/turtle
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v2.13.4
    SOURCE_DIR ../src/external/Catch2
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
