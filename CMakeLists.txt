cmake_minimum_required(VERSION 3.0.0)
project(CatJudge VERSION 0.1.0)

set(IS_DEBUG CMAKE_BUILD_TYPE STREQUAL "Debug")
if (${IS_DEBUG})
  add_definitions(-D__DEBUG__)
endif ()

add_definitions(
        -DCATJUDGE_VERSION=\"0.0.1\"
        -DLOG_PATH=\"./catjudge_log.txt\")

add_executable(main core/main.cpp)
add_executable(prev core/core.cpp)

include_directories(./testlib)
list(APPEND checkers fcmp hcmp lcmp ncmp nyesno rcmp4 rcmp6 rcmp9 wcmp yesno)
foreach(checker IN LISTS checkers)
  add_executable(${checker} testlib/checkers/${checker}.cpp)
endforeach()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()

add_test(NAME Help
         COMMAND $<TARGET_FILE:main> "-h")
add_test(NAME APLUSB
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> ac.cpp 1 "System Error")
add_test(NAME APLUSB_TLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> tle.cpp 1 "Time Limit Exceeded")
add_test(NAME APLUSB_MLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> mle.cpp 1 "Memory Limit Exceeded")
add_test(NAME APLUSB_STACK_ERR
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> stack.cpp 2 "Runtime Error")
add_test(NAME APLUSB_STACK_OK
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> stack.cpp 3 "System Error")
