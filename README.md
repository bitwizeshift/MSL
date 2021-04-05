[![Ubuntu Build Status](https://github.com/bitwizeshift/msl/workflows/Ubuntu/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AUbuntu)
[![macOS Build Status](https://github.com/bitwizeshift/msl/workflows/macOS/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AmacOS)
[![Windows Build Status](https://github.com/bitwizeshift/msl/workflows/Windows/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AWindows)
[![Coverage Status](https://coveralls.io/repos/github/bitwizeshift/msl/badge.svg?branch=master)](https://coveralls.io/github/bitwizeshift/MSL?branch=master)
[![Github Issues](https://img.shields.io/github/issues/bitwizeshift/MSL.svg)](http://github.com/bitwizeshift/msl/issues)
<br>
[![Github Releases](https://img.shields.io/github/v/release/bitwizeshift/msl.svg?include_prereleases)](https://github.com/bitwizeshift/msl/releases)
[![GitHub Sponsors](https://img.shields.io/badge/GitHub-Sponsors-ff69b4)](https://github.com/sponsors/bitwizeshift)

# Memory Support Library

This library is an experimental redesign of the entire C++ Allocator model and
paradigm, written with type-safety and optimizations in mind.

This places as much of an allocation's information into the **type-system** as
possible to provide better hints to the compiler, and to have better static
safety and type disambiguation.

## Teaser

```cpp

// Unaligned dot-product
auto dot(const msl::cell<float[]>& lhs, const msl::cell<float[]>& rhs) -> float { ... }

// Over-aligned dot-product. Use SIMD instructions to optimize this.
auto dot(const msl::cell<float[],64u>& lhs, const msl::cell<float[],64u>& rhs) -> float { ... }
```

## Features

✔ Written in modern <kbd>C++20</kbd> \
✔ Encodes your pointers' reachability and alignment into the **type-system** \
✔ Hints alignment to the compiler for optimal code-generation \
✔ Homogeneous allocator types \
✔ Composable, reusable allocator abstractions \
✔ Adaptable into the standard C++ allocator model

## Quick References

* [🔍 Background](#background) \
  A background on the problem **MSL** solves
* [💾 Installation](doc/installing.md) \
  For a quick guide on how to install/use this in other projects
* [📄 API Reference](https://bitwizeshift.github.io/msl/api/latest/) \
  For doxygen-generated API information
* [🚀 Contributing](.github/CONTRIBUTING.md) \
  How to contribute to the **Result** project
* [💼 Attribution](docs/legal.md) \
  Information about how to attribute this project
* [❓ FAQ](docs/faq.md) \
  A list of frequently asked questions

## Background

TODO

### Comparison with standard allocators

#### Homogeneous Allocators

The C++ allocator requires a lot of rebinding to work correctly, since
allocators are fixed in terms of type `T`. C++17 made the inner machinery a
little nicer by allowing `memory_resource`s that are written in terms of `void*`.
C++20 cleaned this up further by allowing `polymorphic_allocator<>` objects to have
`allocate_bytes()` and `allocate_objects()` -- but this does not change the
fact that `allocator` remains as a `template`.

**MSL** defines exactly 1 type of object for allocators, simply called
`msl::allocator`, which may be built out of any composition of
`memory_resource`s.

#### Better type safety

The C++ allocator model simply distributes pointers (`T*`) for _all_ allocated
objects -- irrespective of whether the pointer refers to an object with an
active lifetime or not.

Additionally, these pointers have no reachability guarantees -- and expressions
like `p - q` are a form of undefined behavior assuming `p` and `q` were both
allocated; but this is allowed from the type system that is used in the
standard.

**MSL** addresses this by using different types to represent the various
states of objects, and each of these come with their own respective limitations
to eliminate undefined behavior:

* `cell` \
  An object that represents an allocated object that does not yet contain a
  lifetime. This type is effectively a dummy wrapper that knows the type and
  reachability, but prevents the user from doing most things that would cause
  undefined behavior. `cell` objects are returned by allocation calls that
  do not construct the objects.

* `active_cell` \
  An object that represents an allocated object that _does_ contain a lifetime.
  This behaves more like a typical pointer, allowing users to dereference and
  access the underlying object(s). `active_cell` objects are returned from
  allocation calls that do construct the objects.

* `unsafe_cell` \
  A `cell` object whose lifetime is unknown. It may represent an `active_cell`
  or a `cell`. It is deemed `unsafe` for this matter, and requires the user
  to manually be aware of its uses and lifetime. `unsafe_cell`s are only ever
  constructed manually by the user. For it to be used with an `allocator`, it
  must be converted back to one of the strongly-typed `cell` or `active_cell`
  objects.

* `memory_block` \
  A contiguous sequence of bytes that is used internally to represent blocks
  inside `memory_resource` objects. These get converted to `cell`s internally.

#### Allocated pointers contain information in their types

The C++ allocator model fails to encode two crucial pieces of detail in
allocated objects: the length of the allocation, and the alignment. This forces
the caller to be aware of the former by storing the size in a disjoint area, and
also causes the latter alignment to be potentially lost to potential
optimizations that over-aligning could bring, since this occurs at runtime.

**MSL** addresses this by encoding as much detail as it can into the types
that are returned by allocators.

* `cell` is actually a `template` on `T` (the type) and its `Align`ment. This
  is done because alignments are almost always known at compile-time since they
  are used to achieve better optimizations for target hardware. Templates help
  the code-generation since they can be statically analyzed and used with
  compiler intrinsics to indicate that these pointers are always aligned.

  * By encoding the alignment in the type, we also gain the ability to
    differentiate overloads based on alignment -- something that can statically
    trigger better code-generation as desired. For example, you can now
    overload:
    ```c++
    // Calls SIMD instructions with no overhead
    void simd_add(active_cell<float[4],64>& out, const active_cell<float[4],64>& to_add);

    // Incurs overhead of unaligned SIMD loads first
    void simd_add(active_cell<float[4]>& out, const active_cell<float[4]>& to_add);
    ```

* `cell` objects of known array bounds, such as `cell<T[N]>`, are always as
  lightweight as a pointer -- since the size is now encoded in the type.

* `cell` objects of unknown array bounds, such as `cell<T[]>`, are the same as
  a `T*` and a `size_t` pair. This is the same effective cost that the C++
  allocator model imposes already indirectly, but couples the sizes together.

#### Support for reallocation and allocation *expansion*

C offers the capability to expand memory regions with `realloc`, since all
objects in C are trivially copyable. C++ cannot offer this guarantee unless
the objects themselves are trivially copyable -- and thus the C++ allocator
model is unable to do this.

**MSL** is able to offer this capability quite easily though, thanks to
differentiating `active_cell` objects from `cell` objects.

* If the allocation being reallocated is a `cell` object, it can always be
  reallocated because no objects are constructed yet in the cell -- so there
  exists nothing to trivially copy.
* If the allocation being reallocated is an `active_cell` object, it can
  always be reallocated if the type is trivially copyable. If the type is not
  trivially copyable, then the active cell range can call the move constructor
  on each object being moved first.

In addition to this, **MSL** also introduces the concept of _resizing_
allocations rather than simply moving to larger allocations. This can allow
allocations to be expanded in-place without the cost of reallocating objects.

This functionality makes a big difference in the implementation of types like
`std::vector`, which can try to `resize_allocation`s first -- which can allow
for an unbounded `vector` using a `monotonic_memory_resource` to grow to the
correct size without needing to `reserve` the space first or incur a
reallocation.

#### Support for fused allocate/construct and destroy/deallocate

The C++ allocator model, prior to C++20, forced users to use a disconnected
`construct` to construct objects in pointers allocated by `allocate`. C++20
introduces a `new_object`/`delete_object` to finally accomplish this, but it
only exists in `polymorphic_allocator<std::byte>` -- which is not particularly
intuitive.

The allocator model in **MSL** is designed from the bottom-up with this idea
in mind -- wherein the `allocator` concept supports:

* `make_object`,
* `make_objects`, and
* `dispose`

Where the former allocates and constructs a single or array of objects
respectively, and the latter destroys and deallocates those objects.

Since **MSL** uses a stronger type for encoding this data, this makes it easy
and intuitive for any user to make use of this.

## FAQ

### What does **MSL** do to prevent UB?

**MSL** encodes active objects, their alignment, and (conditionally) an array's
length into a new _type_ rather than coalescing all of these things into a
generic pointer. This type imposes restrictions on what the user may or may not
do with those values.

As with almost anything in C++, it's always possible for someone hell-bent on
doing something wrong to introduce undefined behavior. However, it is quite
difficult to _unintentionally_ do something wrong with this library.

## License

> Copyright &copy; 2020-2021 Matthew Rodusek
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.
