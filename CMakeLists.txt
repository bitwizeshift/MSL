cmake_minimum_required(VERSION 3.17)

if (DEFINED PROJECT_NAME)
  set(is_subproject TRUE)
endif ()

set(MSL_CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(MSL_CMAKE_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/templates")

set(CMAKE_MODULE_PATH "${MSL_CMAKE_MODULE_PATH}" "${CMAKE_MODULE_PATH}")

option(MSL_ENABLE_UNIT_TESTS "Compile and run the unit tests for this library" ON)

if (NOT CMAKE_TESTING_ENABLED AND MSL_ENABLE_UNIT_TESTS)
  enable_testing()
endif ()

project(MSL
  VERSION "0.1.0"
  DESCRIPTION "An experimental redesign of the C++ Allocator model"
  HOMEPAGE_URL "https://github.com/bitwizeshift/msl"
  LANGUAGES CXX
)

set(MSL_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "Major version of MSL")
set(MSL_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "Minor version of MSL")
set(MSL_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "Patch version of MSL")
set(MSL_VERSION ${PROJECT_VERSION} CACHE INTERNAL "Version of MSL")

##############################################################################
# Targets
##############################################################################

set(header_files
  # Pointers
  include/msl/pointers/raw_pointer.hpp
)

set(source_files
  src/msl/pointers/raw_pointer.cpp
)

add_library(${PROJECT_NAME} ${source_files} ${header_files})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME}
  PUBLIC
    cxx_std_20
)
set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PUBLIC $<INSTALL_INTERFACE:include>
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND
    "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
  # clang-cl does not appear to implement '-pedantic' or 'pedantic-errors',
  # so this case needs to be handled specifically
  add_compile_options(-Wall -Werror -Wextra)

  # Disable the ridiculous compatibility warnings, since it fails on files not
  # ending in newlines
  add_compile_options(-Wno-c++98-compat -Wno-c++98-compat-pedantic)

  # This gives an unbelievable amount of false-positives spuriously. Ignore it.
  add_compile_options(-Wno-unneeded-member-function)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  add_compile_options(-Wall -Werror -Wextra -pedantic -pedantic-errors)
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
  add_compile_options(/Zc:__cplusplus /WX /W4)
endif ()

if (MSL_ENABLE_UNIT_TESTS)
  add_subdirectory("test")
endif ()

##############################################################################
# Installation
##############################################################################

if (is_subproject)
  return()
endif ()

# TODO(bitwize): Add install logic
