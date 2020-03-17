#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>

namespace engine
{
#ifndef ENGINE_NO_BASIC_TYPES
#ifndef ENGINE_NO_U8
	typedef uint8_t u8;
#endif

#ifndef ENGINE_NO_U16
	typedef uint16_t u16;
#endif

#ifndef ENGINE_NO_U32
	typedef uint32_t u32;
#endif

#ifndef ENGINE_NO_U64
	typedef uint64_t u64;
#endif

#ifndef ENGINE_NO_U128
	typedef struct
	{
		u64 data[2];
	} u128;
#endif

#ifndef ENGINE_NO_I8
	typedef int8_t i8;
#endif

#ifndef ENGINE_NO_I16
	typedef int16_t i16;
#endif

#ifndef ENGINE_NO_I32
	typedef int32_t i32;
#endif

#ifndef ENGINE_NO_I64
	typedef int64_t i64;
#endif

#ifndef ENGINE_NO_I128
	typedef struct
	{
		i64 data[2];
	} i128;
#endif

#ifndef ENGINE_NO_F32
	typedef float f32;
#endif

#ifndef ENGINE_NO_F64
	typedef double f64;
#endif

#endif

#define BIT(n) (1<<n)

typedef u32 Rgba32;
typedef std::vector<u32> UnicodeString;

enum class ColorMode
{
	Add,
	Sub,
	Mul
};

enum class AlphaMode
{
	Mask,
	Blend
};


}
