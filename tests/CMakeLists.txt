cmake_minimum_required(VERSION 3.10)

project(regit_tests)
include(cmake/flags.cmake)
include(cmake/utils.cmake)

add_regit_tests(test_variant)
add_regit_tests(test_thread_pool)
add_regit_tests(test_timer)