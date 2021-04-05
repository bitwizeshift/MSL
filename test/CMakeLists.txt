include(CPM)

CPMAddPackage("gh:catchorg/Catch2@2.13.4")

set(source_files
  src/main.cpp
)

add_executable(${PROJECT_NAME}.test
  ${source_files}
)
add_executable(${PROJECT_NAME}::test ALIAS ${PROJECT_NAME}.test)

target_link_libraries(${PROJECT_NAME}.test
  PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
  PRIVATE Catch2::Catch2
)
