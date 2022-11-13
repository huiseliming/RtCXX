#pragma once
#include <type_traits>

template <typename T>
struct TIsReference : public std::false_type
{
};

template <typename T>
struct TIsReference<std::reference_wrapper<T>> : public std::true_type
{
	using WrappedType = T;
};
