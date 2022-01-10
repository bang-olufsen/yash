project(yash)
cmake_minimum_required(VERSION 3.1)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

if (BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}-example example/example.cpp)
    target_include_directories(${PROJECT_NAME}-example PUBLIC include src/external/conioFunctions)
endif()

if (BUILD_TEST)
    add_subdirectory(src)
    add_subdirectory(test)
endif()
