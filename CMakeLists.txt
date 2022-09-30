cmake_minimum_required(VERSION 3.0.0)
project(CatJudge VERSION 0.1.0)

add_definitions(-DCATJUDGE_VERSION=\"0.0.1\")

add_executable(main core/core.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()

add_test(NAME Help
         COMMAND $<TARGET_FILE:main> "-h")
