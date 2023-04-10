# Copyright 2023 Phi-Long Le. All rights reserved.
# Use of this source code is governed by a MIT license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.25)

# Define the project meta information
project(http-server LANGUAGES C CXX)

# ---- Main Project check ----
# determine if this project is built as a subproject (using add_subdirectory) or if it is the main
# project
set(MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MAIN_PROJECT ON)
endif()

set(LONGLP_PROJECT_DIR "${PROJECT_SOURCE_DIR}")
set(LONGLP_PROJECT_SRC_DIR "${LONGLP_PROJECT_DIR}/src")
set(LONGLP_PROJECT_OUTPUT_DIR "${PROJECT_BINARY_DIR}")
set(LONGLP_PROJECT_TEST_DIR "${LONGLP_PROJECT_DIR}/test")
set(LONGLP_PROJECT_EXTERNAL_DIR "${LONGLP_PROJECT_DIR}/external")
set(LONGLP_PROJECT_DATA_DIR "${LONGLP_PROJECT_DIR}/data")
set(LONGLP_PROJECT_GEN_DIR "${LONGLP_PROJECT_OUTPUT_DIR}/generated")
set(LONGLP_PROJECT_DEMO_DIR "${LONGLP_PROJECT_DIR}/demo")
set(LONGLP_PROJECT_BENCHMARK_DIR "${LONGLP_PROJECT_DIR}/benchmark")

# ---- Related tools ----
add_subdirectory(cmake)
longlp_desired_compile_options(LONGLP_DESIRED_COMPILE_OPTIONS)
# set(LONGLP_DESIRED_COMPILE_OPTIONS -Wno-everything)
set(CMAKE_CXX_STANDARD 20)

# ---- Third parties ----
find_package(fmt CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

add_subdirectory(${LONGLP_PROJECT_SRC_DIR})

add_subdirectory(${LONGLP_PROJECT_DEMO_DIR})

# TODO(longlp): recover these

# add_subdirectory(${LONGLP_PROJECT_BENCHMARK_DIR})

# ---- Test ----
include(CTest)
enable_testing()
add_subdirectory(${LONGLP_PROJECT_TEST_DIR})
