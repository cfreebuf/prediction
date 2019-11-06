/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef COMMON_TENSORFLOW_PLATFORM_MACROS_H_
#define COMMON_TENSORFLOW_PLATFORM_MACROS_H_

// A macro to disallow the copy constructor and operator= functions
// This is usually placed in the private: declarations for a class.
#define COMMON_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;         \
  void operator=(const TypeName&) = delete

#if 1
// Compiler attributes
#if (defined(__GNUC__) || defined(__APPLE__)) && !defined(SWIG)
// Compiler supports GCC-style attributes
#define COMMON_ATTRIBUTE_NORETURN __attribute__((noreturn))
#define COMMON_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#define COMMON_ATTRIBUTE_NOINLINE __attribute__((noinline))
#define COMMON_ATTRIBUTE_UNUSED __attribute__((unused))
#define COMMON_ATTRIBUTE_COLD __attribute__((cold))
#define COMMON_ATTRIBUTE_WEAK __attribute__((weak))
#define COMMON_PACKED __attribute__((packed))
#define COMMON_MUST_USE_RESULT __attribute__((warn_unused_result))
#define COMMON_PRINCOMMON_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__printf__, string_index, first_to_check)))
#define COMMON_SCANF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__scanf__, string_index, first_to_check)))
#elif defined(_MSC_VER)
// Non-GCC equivalents
#define COMMON_ATTRIBUTE_NORETURN __declspec(noreturn)
#define COMMON_ATTRIBUTE_ALWAYS_INLINE __forceinline
#define COMMON_ATTRIBUTE_NOINLINE
#define COMMON_ATTRIBUTE_UNUSED
#define COMMON_ATTRIBUTE_COLD
#define COMMON_ATTRIBUTE_WEAK
#define COMMON_MUST_USE_RESULT
#define COMMON_PACKED
#define COMMON_PRINCOMMON_ATTRIBUTE(string_index, first_to_check)
#define COMMON_SCANF_ATTRIBUTE(string_index, first_to_check)
#else
// Non-GCC equivalents
#define COMMON_ATTRIBUTE_NORETURN
#define COMMON_ATTRIBUTE_ALWAYS_INLINE
#define COMMON_ATTRIBUTE_NOINLINE
#define COMMON_ATTRIBUTE_UNUSED
#define COMMON_ATTRIBUTE_COLD
#define COMMON_ATTRIBUTE_WEAK
#define COMMON_MUST_USE_RESULT
#define COMMON_PACKED
#define COMMON_PRINCOMMON_ATTRIBUTE(string_index, first_to_check)
#define COMMON_SCANF_ATTRIBUTE(string_index, first_to_check)
#endif

// Control visiblity outside .so
#if defined(_WIN32)
#ifdef COMMON_COMPILE_LIBRARY
#define COMMON_EXPORT __declspec(dllexport)
#else
#define COMMON_EXPORT __declspec(dllimport)
#endif  // COMMON_COMPILE_LIBRARY
#else
#define COMMON_EXPORT __attribute__((visibility("default")))
#endif  // _WIN32

#ifdef __has_builtin
#define COMMON_HAS_BUILTIN(x) __has_builtin(x)
#else
#define COMMON_HAS_BUILTIN(x) 0
#endif

// Compilers can be told that a certain branch is not likely to be taken
// (for instance, a CHECK failure), and use that information in static
// analysis. Giving it this information can help it optimize for the
// common case in the absence of better information (ie.
// -fprofile-arcs).
//
// We need to disable this for GPU builds, though, since nvcc8 and older
// don't recognize `__builtin_expect` as a builtin, and fail compilation.
#if (!defined(__NVCC__)) && \
    (COMMON_HAS_BUILTIN(__builtin_expect) || (defined(__GNUC__) && __GNUC__ >= 3))
#define COMMON_PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define COMMON_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#else
#define COMMON_PREDICT_FALSE(x) (x)
#define COMMON_PREDICT_TRUE(x) (x)
#endif

// A macro to disallow the copy constructor and operator= functions
// This is usually placed in the private: declarations for a class.
#define COMMON_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;         \
  void operator=(const TypeName&) = delete

// The COMMON_ARRAYSIZE(arr) macro returns the # of elements in an array arr.
//
// The expression COMMON_ARRAYSIZE(a) is a compile-time constant of type
// size_t.
#define COMMON_ARRAYSIZE(a)         \
  ((sizeof(a) / sizeof(*(a))) / \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L || \
    (defined(_MSC_VER) && _MSC_VER >= 1900)
// Define this to 1 if the code is compiled in C++11 mode; leave it
// undefined otherwise.  Do NOT define it to 0 -- that causes
// '#ifdef LANG_CXX11' to behave differently from '#if LANG_CXX11'.
#define LANG_CXX11 1
#endif

#if defined(__clang__) && defined(LANG_CXX11) && defined(__has_warning)
#if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define COMMON_FALLTHROUGH_INTENDED [[clang::fallthrough]]  // NOLINT
#endif
#endif

#ifndef COMMON_FALLTHROUGH_INTENDED
#define COMMON_FALLTHROUGH_INTENDED \
  do {                          \
  } while (0)
#endif

#endif  // endif 0

#endif  // COMMON_TENSORFLOW_PLATFORM_MACROS_H_
