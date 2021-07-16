project(external)

include(ExternalProject)
cmake_minimum_required(VERSION 3.1)

ExternalProject_Add(
    turtle
    GIT_REPOSITORY https://github.com/mat007/turtle
    GIT_TAG bfd1701fcbbb77258ce82dec5a755ff969cbadd3
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

ExternalProject_Add(
    conioFunctions
    GIT_REPOSITORY https://github.com/yogesh-hackx/conioFunctions
    GIT_TAG 1f57ea84d6e5a0ff5c0624c2718c3535cc74738e
    SOURCE_DIR ../src/external/conioFunctions
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
