#pragma once
#include <type_traits>

template <typename T>
struct TIsVector : public std::false_type
{
	using ElementType = void;
};

template <typename T>
struct TIsVector<std::vector<T>> : public std::true_type
{
	using ElementType = T;
};
