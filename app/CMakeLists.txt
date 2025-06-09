# This is the CMake file specifying compiler options for development and release
# executables.
cmake_minimum_required(VERSION 3.10)
project(awns-rpi5 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
set(COMPILER_FLAGS        "-Wall -Wextra -pedantic -Werror")
# set(COMPILER_FLAGS        "-Wall -Wextra -pedantic ")
set(ADDRESS_SANITIZER_FL   "-fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(THREAD_SANITIZER_FL    "-fsanitize=thread,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(DEBUG_FLAGS            "-g")
set(OPTIMIZATION_FLAGS     "-O3")

if(CMAKE_BUILD_TYPE STREQUAL "memory")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${ADDRESS_SANITIZER_FL} ${DEBUG_FLAGS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "thread")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${THREAD_SANITIZER_FL} ${DEBUG_FLAGS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "release")
  set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} ${OPTIMIZATION_FLAGS}")
endif()

# Coverage support
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Building with coverage flags…")
  add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_link_options(-lclang_rt.profile_osx)
  else()
    add_link_options(-lclang_rt.profile)
  endif()
endif()

# Project headers
include_directories(${CMAKE_SOURCE_DIR})

# Source files
file(GLOB SOURCES_CPP "${CMAKE_SOURCE_DIR}/*.cpp")
file(GLOB SOURCES_C   "${CMAKE_SOURCE_DIR}/*.c")

# Executable
add_executable(awns-rpi5 ${SOURCES_CPP} ${SOURCES_C})

# GPS library via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPS REQUIRED libgps)
target_include_directories(awns-rpi5 SYSTEM PRIVATE ${GPS_INCLUDE_DIRS})
target_link_libraries(awns-rpi5 PRIVATE ${GPS_LDFLAGS})

# Concorde (assumes system-wide install: /usr/local/include, /usr/local/lib)
find_library(CONCORDE_LIB concorde PATHS /usr/local/lib REQUIRED)
find_path(CONCORDE_INCLUDE_DIR tsp.h PATHS /usr/local/include REQUIRED)
target_include_directories(awns-rpi5 SYSTEM PRIVATE ${CONCORDE_INCLUDE_DIR})
target_link_libraries(awns-rpi5 PRIVATE ${CONCORDE_LIB})

# nlohmann_json (header-only JSON library)
find_package(nlohmann_json REQUIRED)
target_link_libraries(awns-rpi5 PRIVATE nlohmann_json::nlohmann_json)

# Show final library/header results
message(STATUS "Concorde library: ${CONCORDE_LIB}")
message(STATUS "Concorde include: ${CONCORDE_INCLUDE_DIR}")
message(STATUS "nlohmann_json version: ${nlohmann_json_VERSION}")
