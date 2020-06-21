cmake_minimum_required(VERSION 3.15)

if (PROJECT_NAME)
  set(IS_SUBPROJECT TRUE)
endif ()

set(MSL_CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(MSL_CMAKE_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/templates")

set(CMAKE_MODULE_PATH "${MSL_CMAKE_MODULE_PATH}" "${CMAKE_MODULE_PATH}")

option(MSL_COMPILE_UNIT_TESTS "Compile and run the unit tests for this library" ON)

if (NOT CMAKE_TESTING_ENABLED AND MSL_COMPILE_UNIT_TESTS)
  enable_testing()
endif ()

project(MSL
  VERSION "1.1.0"
  LANGUAGES CXX
)

set(MSL_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "Major version of MSL")
set(MSL_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "Minor version of MSL")
set(MSL_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "Patch version of MSL")
set(MSL_VERSION ${PROJECT_VERSION} CACHE INTERNAL "Version of MSL")

# If using conan, only set the find paths. This project is trying to be
# fully CMake
if (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake")
  include("${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake")
  conan_set_find_paths()
endif ()

##############################################################################
# Targets
##############################################################################

set(header_files
  include/msl/cells/cell.hpp
  include/msl/utilities/not_null.hpp
  include/msl/utilities/quantity.hpp
  include/msl/utilities/digital_quantity.hpp
  include/msl/utilities/assert.hpp
  include/msl/utilities/intrinsics.hpp
  include/msl/utilities/uninitialized_storage.hpp
  include/msl/allocator.hpp
  include/msl/utilities/attributes.hpp include/msl/cells/active_cell.hpp include/msl/cells/cell_activator.hpp include/msl/cells/unsafe_cell.hpp)

set(source_files
  src/msl/library.cpp
  include/msl/library.h
  include/msl/cells/detail/byte_or_quantity.hpp)

add_library(${PROJECT_NAME} ${source_files} ${header_files})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME}
  PUBLIC
    cxx_std_17
)
set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  INTERFACE $<INSTALL_INTERFACE:include>
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
  add_compile_options(/Zc:__cplusplus)
endif ()

include(AddSelfContainmentTest)

if (MSL_COMPILE_UNIT_TESTS)
  add_self_containment_test(${PROJECT_NAME}.SelfContainmentTest
    TARGET ${PROJECT_NAME}
    HEADERS ${header_files}
  )

#  add_subdirectory("test")
endif ()

##############################################################################
# Installation
##############################################################################

if (IS_SUBPROJECT)
  return()
endif ()

# TODO(bitwize): Add install logic