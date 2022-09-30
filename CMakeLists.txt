cmake_minimum_required(VERSION 3.0.0)
project(CatJudge VERSION 0.1.0)

add_definitions(
        -DCATJUDGE_VERSION=\"0.0.1\"
        -DLOG_PATH=\"./catjudge_log.txt\")

add_executable(main core/main.cpp)
add_executable(prev core/core.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()

add_test(NAME Help
         COMMAND $<TARGET_FILE:main> "-h")
add_test(NAME APLUSB
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> ac.cpp 1)
add_test(NAME APLUSB_TLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> tle.cpp 1)
add_test(NAME APLUSB_MLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> mle.cpp 1)
