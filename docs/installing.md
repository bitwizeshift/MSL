# Installing

**MSL** is defined using modern CMake, and as such can be consumed in a number
of different ways:

* Adding via subdirectory
* Installing with `cmake` and finding it with `find_package`
* using [`CPM`][CPM]

In addition to the basic CMake consumption, this project also provides a
`conanfile` for [`conan`][conan] integration, which offers the standard CMake
integration.

## CMake

### Adding via subdirectory

A simple means of integration is to clone this repository as a
submodule / subtree, and added with `add_subdirectory`:

```cmake
# Assuming installed to 'external/msl'
add_subdirectory(external/msl)

# ...

add_executable(Foo ...)
target_link_libraries(Foo PRIVATE MSL::MSL)
```

### Installing with CMake

The project can be installed *locally* on the system and found via
`find_package`. For example:

```bash
mkdir build && cd build

# Configure and build it
cmake ..
cmake --build .

# Install it
cmake --build . --target install
```

Then in the consuming `CMakeLists.txt`:

```cmake
find_package(MSL REQUIRED)

# ...

add_executable(Foo ...)
target_link_libraries(Foo PRIVATE MSL::MSL)
```

### Using CPM

If you're using [`CPM`][CPM], you can easily make use of this project with a
simple reference:

```cmake
include(CPM)

CPMAddPackage("gh:bitwizeshift/MSL@master")

# ...

add_executable(Foo ...)
target_link_libraries(Foo PRIVATE MSL::MSL)
```

<!-- ----------------------------------------------------------------------- -->

[CPM]: https://github.com/cpm-cmake/CPM.cmake
[conan]: https://conan.io
