cmake_minimum_required(VERSION 3.0.0)
project(CatJudge VERSION 0.3.6)

set(IS_DEBUG CMAKE_BUILD_TYPE STREQUAL "Debug")
if (${IS_DEBUG})
    add_definitions(-D__DEBUG__)
endif ()

set(LOG_PATH .)
set(DEFAULT_CHECKER lcmp)
set(CHECKER_ROOT ${CMAKE_CURRENT_BINARY_DIR})

if (DEFINED ENV{LOG_PATH})
    set(LOG_PATH $ENV{LOG_PATH})
endif ()
if (DEFINED ENV{DEFAULT_CHECKER})
    set(DEFAULT_CHECKER $ENV{DEFAULT_CHECKER})
endif ()
if (DEFINED ENV{CHECKER_ROOT})
    set(CHECKER_ROOT $ENV{CHECKER_ROOT})
endif ()

add_definitions(
        -DCATJUDGE_VERSION=\"${CMAKE_PROJECT_VERSION}\"
        -DLOG_PATH=\"${LOG_PATH}\"
        -DDEFAULT_CHECKER=\"${DEFAULT_CHECKER}\"
        -DCHECKER_ROOT=\"${CHECKER_ROOT}\")

add_executable(main core/main.cpp)

include_directories(./testlib)
list(APPEND checkers fcmp hcmp lcmp ncmp nyesno rcmp4 rcmp6 rcmp9 wcmp yesno)
foreach (checker IN LISTS checkers)
    add_executable(${checker} testlib/checkers/${checker}.cpp)
endforeach ()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Test

include(CTest)
enable_testing()

# Set python path
set(python python)
# Set test runner script path
set(runner ../core/test/run.py)

add_test(NAME Help COMMAND $<TARGET_FILE:main> "-h")
add_test(NAME APLUSB_AC
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> ac.cpp 1 "Accepted")
add_test(NAME APLUSB_AC_RETURN
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> ac_return.cpp 1 "RuntimeError")
add_test(NAME APLUSB_WA
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> wa.cpp 2 "WrongAnswer")
add_test(NAME APLUSB_IO
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> io.cpp 2 "IdlenessLimitExceeded")
add_test(NAME APLUSB_RE
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> re.cpp 2 "RuntimeError")
add_test(NAME APLUSB_TLE
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> tle.cpp 1 "TimeLimitExceeded")
add_test(NAME APLUSB_MLE
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> mle.cpp 1 "MemoryLimitExceeded")
add_test(NAME APLUSB_BIG_STACK
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> big_stack.cpp 2 "MemoryLimitExceeded")
add_test(NAME APLUSB_SMALL_STACK
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> small_stack.cpp 2 "Accepted")
add_test(NAME APLUSB_JAVA
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> ac.java 1 "Accepted")

# Security
add_test(NAME FORK_BOMB
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> fork.cpp 3 "RuntimeError")
add_test(NAME FORBIDDEN_FS
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> fs.cpp 3 "RuntimeError")
add_test(NAME READ_ENV
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> env.cpp 1 "RuntimeError")
add_test(NAME APLUSB_JAVA_FS
        COMMAND ${python} ${runner} $<TARGET_FILE:main> $<TARGET_FILE:lcmp> fs.java 1 "WrongAnswer")
