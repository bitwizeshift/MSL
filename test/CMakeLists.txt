include(CPM)

CPMAddPackage("gh:catchorg/Catch2@2.13.4")

set(source_files
  src/main.cpp
  src/pointers/raw_pointer.test.cpp
  src/pointers/pointer_like.test.cpp
  src/pointers/traversable_pointer.test.cpp
  src/pointers/nullable_pointer.test.cpp
  src/pointers/not_null.test.cpp
)

add_executable(${PROJECT_NAME}.test
  ${source_files}
)
add_executable(${PROJECT_NAME}::test ALIAS ${PROJECT_NAME}.test)

target_link_libraries(${PROJECT_NAME}.test
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
  PRIVATE Catch2::Catch2
)

##############################################################################
# CTest
##############################################################################

add_test(${PROJECT_NAME}.test ${PROJECT_NAME}.test)
