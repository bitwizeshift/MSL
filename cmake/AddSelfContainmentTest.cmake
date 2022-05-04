# Self Containment Test
# =====================
#
# Adds support for cmake-generated source files, one per header, to ensure that
# all headers for a library's public API can be included independently

#[[
  The MIT License (MIT)

  Copyright (c) 2022 Matthew Rodusek

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]

#.rst:
# AddSelfContainmentTest
# ----------------------
#
# Creates a C or C++ library that includes each specified header file
# independently to ensure that each header carries no expected ordering.
#
# This is used to avoid accidental dependencies based on the order of
# inclusion.
#
# ::
#
#     add_self_containment_test(
#       <name>
#       <header0> [headers...]
#     )
#
#     add_self_containment_test(
#       <name>
#       ALIAS <target>
#     )
#
#     <name>        - The name of the target to create
#     <target>      - A target to alias for alias targets
#     [headers]...  - List of headers to compile
#
function(add_self_containment_test name)

  ############################### Setup output ###############################

  cmake_parse_arguments("CONTAINMENT" "" "ALIAS" "" ${ARGN})

  if (TARGET "${name}")
    message(FATAL_ERROR "Specified target name already exists as a valid CMake target.")
  endif()

  set(header_files ${CONTAINMENT_UNPARSED_ARGUMENTS})

  if (CONTAINMENT_ALIAS AND CONTAINMENT_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
      "Unexpected arguments for alias target provided: ${CONTAINMENT_UNPARSED_ARGUMENTS}"
    )
  elseif (CONTAINMENT_ALIAS)
    add_library("${name}" ALIAS "${CONTAINMENT_ALIAS}")
    return ()
  endif ()

  set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}")

  ############################### Create files ###############################

  set(source_files)
  foreach (header IN LISTS header_files)

    cmake_path(GET header PARENT_PATH path_segment)
    cmake_path(ABSOLUTE_PATH header NORMALIZE OUTPUT_VARIABLE absolute_header)
    cmake_path(GET header EXTENSION LAST_ONLY path_extension)

    if (path_extension STREQUAL ".h")
      set(extension "c")
    elseif (path_extension STREQUAL ".hpp")
      set(extension "cpp")
    endif ()

    set(output_path "${output_dir}/${header}.${extension}")

    if (NOT EXISTS "${output_path}" OR "${absolute_header}" IS_NEWER_THAN "${output_path}")
      file(WRITE "${output_path}" "#include \"${absolute_header}\" // IWYU pragma: keep\n")
    endif()

    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${output_path}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${output_path}")

    list(APPEND source_files "${output_path}")

  endforeach ()

  ###################### Create self-containment Target ######################

  add_library("${name}" OBJECT
    ${source_files}
    ${header_files}
  )
  set_target_properties("${name}"
    PROPERTIES
      FOLDER "Containment Tests"
  )

endfunction ()
