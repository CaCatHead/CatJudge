cmake_minimum_required(VERSION 3.0.0)
project(CatJudge VERSION 0.1.0)

set(IS_DEBUG CMAKE_BUILD_TYPE STREQUAL "Debug")
if (${IS_DEBUG})
  add_definitions(-D__DEBUG__)
endif ()

set(PROJECT_VERSION 0.0.1)
set(LOG_PATH ./catjudge_log.txt)
set(DEFAULT_CHECKER ${CMAKE_CURRENT_BINARY_DIR}/lcmp)

add_definitions(
        -DCATJUDGE_VERSION=\"${PROJECT_VERSION}\"
        -DLOG_PATH=\"${LOG_PATH}\"
        -DDEFAULT_CHECKER=\"${DEFAULT_CHECKER}\")

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
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> $<TARGET_FILE:lcmp> ac.cpp 1 "System Error")
add_test(NAME APLUSB_TLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> $<TARGET_FILE:lcmp> tle.cpp 1 "Time Limit Exceeded")
add_test(NAME APLUSB_MLE
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> $<TARGET_FILE:lcmp> mle.cpp 1 "Memory Limit Exceeded")
add_test(NAME APLUSB_STACK_ERR
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> $<TARGET_FILE:lcmp> stack.cpp 2 "Runtime Error")
add_test(NAME APLUSB_STACK_OK
         COMMAND python ../core/test/run.py $<TARGET_FILE:main> $<TARGET_FILE:lcmp> stack.cpp 3 "System Error")
