set(MODULE_NAME test-yash)

add_executable(${MODULE_NAME} TestYash.cpp)
target_link_libraries(${MODULE_NAME} boost_unit_test_framework catch turtle yash)

target_compile_options(${MODULE_NAME} PUBLIC
    -DBOOST_BIND_GLOBAL_PLACEHOLDERS
)

add_test(${MODULE_NAME} ${MODULE_NAME})
