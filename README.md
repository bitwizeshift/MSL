[![Ubuntu Build Status](https://github.com/bitwizeshift/msl/workflows/Ubuntu/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AUbuntu)
[![macOS Build Status](https://github.com/bitwizeshift/msl/workflows/macOS/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AmacOS)
[![Windows Build Status](https://github.com/bitwizeshift/msl/workflows/Windows/badge.svg?branch=master)](https://github.com/bitwizeshift/msl/actions?query=workflow%3AWindows)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e163a49b3b2e4f1e953c32b7cbbb2f28)](https://www.codacy.com/gh/bitwizeshift/msl/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=bitwizeshift/result&amp;utm_campaign=Badge_Grade)
[![Coverage Status](https://coveralls.io/repos/github/bitwizeshift/msl/badge.svg?branch=master)](https://coveralls.io/github/bitwizeshift/result?branch=master)
[![Github Issues](https://img.shields.io/github/issues/bitwizeshift/result.svg)](http://github.com/bitwizeshift/msl/issues)
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
