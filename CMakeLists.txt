project(yash)
cmake_minimum_required(VERSION 3.1)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

if (BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}-example src/example/example.cpp)
    target_include_directories(${PROJECT_NAME}-example PUBLIC include src/external/conioFunctions)
endif()

if (BUILD_TESTING)
    if (BUILD_EXTERNAL)
        add_subdirectory(src)
    endif()
    add_subdirectory(test)
endif()
