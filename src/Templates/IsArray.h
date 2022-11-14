#pragma once

template<typename InElementType, typename InAllocatorType>
class TArray {};

template <typename T> struct TIsTArray { enum { Value = false }; };

template <typename InElementType, typename InAllocatorType> struct TIsTArray<               TArray<InElementType, InAllocatorType>> { enum { Value = true }; };
template <typename InElementType, typename InAllocatorType> struct TIsTArray<const          TArray<InElementType, InAllocatorType>> { enum { Value = true }; };
template <typename InElementType, typename InAllocatorType> struct TIsTArray<      volatile TArray<InElementType, InAllocatorType>> { enum { Value = true }; };
template <typename InElementType, typename InAllocatorType> struct TIsTArray<const volatile TArray<InElementType, InAllocatorType>> { enum { Value = true }; };

template<typename T>
struct TArrayTraitsBase
{
    using ElementType = void;
    using AllocatorType = void;
};

template<typename InElementType, typename InAllocatorType>
struct TArrayTraitsBase<TArray<InElementType, InAllocatorType>>
{
    using ElementType = InElementType;
    using AllocatorType = InAllocatorType;
};

template<typename T>
struct TArrayTraits : public TArrayTraitsBase<T>
{

};
