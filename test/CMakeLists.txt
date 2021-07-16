set(MODULE_NAME test-yash)

add_executable(${MODULE_NAME} TestYash.cpp)
target_link_libraries(${MODULE_NAME} catch turtle yash)

add_test(${MODULE_NAME} ${MODULE_NAME})
