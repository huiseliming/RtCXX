#pragma once
#include "../Fwd.h"

template <i32 IndexSize>
struct TBytesToSizeType
{
	static_assert(IndexSize, "Unsupported allocator index size.");
};
template <>
struct TBytesToSizeType<1>
{
	using Type = U8;
};
template <>
struct TBytesToSizeType<2>
{
	using Type = U16;
};
template <>
struct TBytesToSizeType<4>
{
	using Type = U32;
};
template <>
struct TBytesToSizeType<8>
{
	using Type = U64;
};

using PointerSizeType = TBytesToSizeType<sizeof(void*)>::Type;
