# This is the CMake file specifying compiler options for development and release
# executables.

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(awns-rpi5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(COMPILER_FLAGS "-Wall -Wextra -pedantic -Werror")
set(ADDRESS_SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(THREAD_SANITIZER_FLAGS "-fsanitize=thread,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(DEBUG_FLAGS "-g")
set(OPTIMIZATION_FLAGS "-O3")
# set(IGNORE_ERROR_FLAGS "-Wno-c99-designator")
# set(IGNORE_ERROR_FLAGS "-Wnested-anon-types")

if (CMAKE_BUILD_TYPE STREQUAL "memory")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${ADDRESS_SANITIZER_FLAGS} ${DEBUG_FLAGS} ${IGNORE_ERROR_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL "thread")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${THREAD_SANITIZER_FLAGS} ${DEBUG_FLAGS} ${IGNORE_ERROR_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL "release")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${OPTIMIZATION_FLAGS} ${IGNORE_ERROR_FLAGS}")
endif()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if (ENABLE_COVERAGE)
  message(STATUS "Building with coverage flags...")
  add_compile_options(-fprofile-instr-generate -fcoverage-mapping)

  # Detect and link the correct `clang_rt.profile` library for the system
  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    set(COVERAGE_LIB "-lclang_rt.profile_osx")
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Linux
    set(COVERAGE_LIB "-lclang_rt.profile")
  else()
    message(WARNING "Unknown system, trying generic coverage library")
    set(COVERAGE_LIB "-lclang_rt.profile")
  endif()
  add_link_options(${COVERAGE_LIB})
endif()

# Headers
include_directories(${CMAKE_SOURCE_DIR})
# Implementation
file(GLOB SOURCES_CPP
  ${CMAKE_SOURCE_DIR}/*.cpp)
file(GLOB SOURCES_C
  ${CMAKE_SOURCE_DIR}/*.c)

# Link source files for executable
add_executable(awns-rpi5 ${SOURCES_CPP} ${SOURCES_C})
# add_executable(awns-rpi5 ${SOURCES_CPP})

# Ignore certain warnings in order to use libraries
# target_compile_options(awns-rpi5 PRIVATE
#   -Wno-nested-anon-types
#   -Wno-error=nested-anon-types
# )

# Link libgps using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPS REQUIRED libgps)

target_include_directories(awns-rpi5 SYSTEM PRIVATE ${GPS_INCLUDE_DIRS})
target_link_libraries(awns-rpi5 PRIVATE ${GPS_LDFLAGS})
