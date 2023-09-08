#pragma once
#include <stdbool.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef bool Bool;

#define True true
#define False false

// Inlining
#if defined(__clang__) || defined(__gcc__)
#define RINLINE __attribute__((always_inline)) inline
#define RNOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define RINLINE __forceinline
#define RNOINLINE __declspec(noinline)
#else
#define RINLINE static inline
#define RNOINLINE
#endif

#define INF 0x7FFFFFFF
#define FLT_MAX 3.402823466e+38F

#define ATTR_PACKED __attribute__((packed))