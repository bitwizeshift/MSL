cmake_minimum_required(VERSION 3.17)

if (DEFINED PROJECT_NAME)
  set(is_subproject TRUE)
endif ()

###############################################################################
# Project Options
###############################################################################

option(BUILD_SHARED_LIBS "Build all libraries as a shared library" OFF)
option(MSL_ENABLE_UNIT_TESTS "Compile and run the unit tests for this library" ON)
option(MSL_DISABLE_STRICT_MODE "Disables strict/esoteric C++ requirements" OFF)
option(MSL_PRESET_CONFIGURATION "Option set when using --preset argument" OFF)

###############################################################################
# Settings
###############################################################################

set(MSL_CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(MSL_CMAKE_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/templates")

set(CMAKE_MODULE_PATH "${MSL_CMAKE_MODULE_PATH}" "${CMAKE_MODULE_PATH}")

###############################################################################
# Project Definition
###############################################################################

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

include(BuildConfigurations)
include(AddSelfContainmentTest)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/data/config.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/msl/config.hpp"
  @ONLY
)

set(generated_header_files
  ${CMAKE_CURRENT_BINARY_DIR}/include/msl/config.hpp
)

set(header_files
  # Utilities
  include/msl/utilities/assert.hpp
  include/msl/utilities/intrinsics.hpp
  include/msl/utilities/source_location.hpp
  include/msl/utilities/dynamic_storage.hpp

  # Quantities
  include/msl/quantities/address.hpp
  include/msl/quantities/address_io.hpp
  include/msl/quantities/alignment.hpp
  include/msl/quantities/digital_quantity.hpp
  include/msl/quantities/quantity.hpp

  # Pointers
  include/msl/pointers/not_null.hpp
  include/msl/pointers/nullable_pointer.hpp
  include/msl/pointers/pointer_like.hpp
  include/msl/pointers/pointer_utilities.hpp
  include/msl/pointers/raw_pointer.hpp
  include/msl/pointers/tagged_ptr.hpp
  include/msl/pointers/traversable_pointer.hpp
  include/msl/pointers/traversal_utilities.hpp
  include/msl/pointers/intrusive_pointer_stack.hpp
  include/msl/pointers/lifetime_utilities.hpp
  include/msl/pointers/unaligned_utilities.hpp

  # Memory
  include/msl/memory/virtual_memory.hpp

  # Blocks
  include/msl/blocks/memory_block.hpp

  # Cells
  include/msl/cells/cell.hpp
)

set(source_files
  # Quantities
  src/msl/quantities/alignment.cpp

  # Pointers
  src/msl/pointers/not_null.cpp
  src/msl/pointers/intrusive_pointer_stack.cpp

  # Memory
  src/msl/memory/virtual_memory.cpp

  # Cells
  src/msl/cells/cell.cpp
)

if (WIN32)
  list(APPEND source_files
    src/msl/memory/win32/virtual_memory_impl.cpp
  )
elseif (APPLE OR UNIX)
  list(APPEND source_files
    src/msl/memory/posix/virtual_memory_impl.cpp
  )
else ()
  list(APPEND source_files
    src/msl/memory/default/virtual_memory_impl.cpp
  )
endif ()

add_library(${PROJECT_NAME}
  ${source_files}
  ${header_files}
  ${generated_header_files}
)
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

add_self_containment_test(${PROJECT_NAME}.containment
  ${header_files}
  ${generated_header_files}
)
add_self_containment_test(${PROJECT_NAME}::${PROJECT_NAME}.containment
  ALIAS ${PROJECT_NAME}.containment
)
target_link_libraries(${PROJECT_NAME}.containment
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)

# Ensure that consumers don't trigger errors in our headers when included
# recursively
if (is_subproject)
  set(header_visibility SYSTEM)
else ()
  set(header_visibility)
endif ()

target_include_directories(${PROJECT_NAME}
  ${header_visibility}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_include_directories(${PROJECT_NAME}
  SYSTEM PUBLIC $<INSTALL_INTERFACE:include>
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

include(PatchCompileCommands)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(MSL_CMAKE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# Create package configuration
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY
    SameMajorVersion
)
configure_file(
  "${MSL_CMAKE_TEMPLATE_PATH}/PackageConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  @ONLY
)

# Install targets / configuration
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY
  PUBLIC_HEADER
)
install(
  DIRECTORY "include/"
  TYPE INCLUDE
)

install(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "${MSL_CMAKE_CONFIG_DESTINATION}"
)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION "${MSL_CMAKE_CONFIG_DESTINATION}"
)
