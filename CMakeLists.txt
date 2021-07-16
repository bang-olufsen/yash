project(yash)
cmake_minimum_required(VERSION 3.1)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

if (BUILD_YASH_MAIN)
    add_library(${PROJECT_NAME}-main src/main.cpp)
    target_include_directories(${PROJECT_NAME}-main PUBLIC include)
endif()

if (BUILD_TESTING)
    if (BUILD_EXTERNAL)
        add_subdirectory(src)
    endif()
    add_subdirectory(test)
endif()
