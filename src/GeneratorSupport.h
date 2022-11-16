#pragma once
#include "RtCXX.h"
#include "Templates/FunctionTraits.h"
#include "Templates/IsArray.h"
#include "Templates/Traits.h"

RTCXX_NAMESPACE_BEGIN

RTCXX_API void SetInheritanceRelationship(CMetaClass* InMetaClass, const char* InBaseClassName, CController* InController);
RTCXX_API void SetStandardClassPtrProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController = nullptr);
RTCXX_API void SetStandardClassProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController = nullptr);

template <typename T>
concept CRHasConstAttrs = requires(T * Class)
{
	Class->ConstAttrs;
};

template <typename UnqiueAnonymousType, size_t Index, size_t Size>
FORCEINLINE void ForEachSetConstAttr(CMetadata* InMetadata)
{
	if constexpr (Size > Index)
	{
		constexpr auto Attr = UnqiueAnonymousType::ConstAttrs[Index];
		InMetadata->SetAttr(Attr.first, std::get<Attr.second.index()>(Attr.second));
		ForEachSetConstAttr<UnqiueAnonymousType, Index + 1, Size>(InMetadata);
	}
}

template <typename UnqiueAnonymousType>
FORCEINLINE void StaticSetConstAttrs(CMetadata* InMetadata)
{
	if constexpr (CRHasConstAttrs<UnqiueAnonymousType>)
	{
		constexpr auto ConstAttrsSize = UnqiueAnonymousType::ConstAttrs.size();
		ForEachSetConstAttr<UnqiueAnonymousType, 0, ConstAttrsSize>(InMetadata);
	}
}

template <typename UnqiueAnonymousType, typename T>
FORCEINLINE auto StaticCreateUniqueClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags, const char* InBaseClassName, const char* InInterfaceNames[], CController* InController)
{
	assert(InController);
	static TClass<T> LClass(InOwner, InName, InClassFlags);
	if (InBaseClassName)
	{
		SetInheritanceRelationship(&LClass, InBaseClassName, InController);
	}
	// const char** InterfaceName = InInterfaceNames
	// while (*InterfaceName)
	//{
	//	InterfaceName?
	//	InterfaceName++;
	// }
	StaticSetConstAttrs<UnqiueAnonymousType>(&LClass);
	InController->RegisterMetadata(&LClass);
	return &LClass;
}

template <typename T>
struct TChooseArithmeticProperty
{
};
template <>
struct TChooseArithmeticProperty<bool>
{
	using Type = CBoolProperty;
};
template <>
struct TChooseArithmeticProperty<I8>
{
	using Type = CI8Property;
};
template <>
struct TChooseArithmeticProperty<I16>
{
	using Type = CI16Property;
};
template <>
struct TChooseArithmeticProperty<I32>
{
	using Type = CI32Property;
};
template <>
struct TChooseArithmeticProperty<I64>
{
	using Type = CI64Property;
};
template <>
struct TChooseArithmeticProperty<U8>
{
	using Type = CU8Property;
};
template <>
struct TChooseArithmeticProperty<U16>
{
	using Type = CU16Property;
};
template <>
struct TChooseArithmeticProperty<U32>
{
	using Type = CU32Property;
};
template <>
struct TChooseArithmeticProperty<U64>
{
	using Type = CU64Property;
};
template <>
struct TChooseArithmeticProperty<F32>
{
	using Type = CF32Property;
};
template <>
struct TChooseArithmeticProperty<F64>
{
	using Type = CF64Property;
};

template <typename T, class Enabled = void>
struct TChooseProperty
{
	using Type = void;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<std::is_same_v<void, T>>>
{
	using Type = CMetaProperty;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<std::is_arithmetic_v<T>>>
{
	using Type = typename TChooseArithmeticProperty<T>::Type;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<std::is_same_v<std::string, T>>>
{
	using Type = CStrProperty;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<!std::is_same_v<std::string, T>&& std::is_class_v<T>>>
{
	using Type = CClassProperty;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<std::is_reference_v<T>>>
{
	using PointerToType = std::remove_reference_t<T>;
	using PointerToPropType = typename TChooseProperty<PointerToType>::Type;
	using Type = std::conditional_t<std::is_base_of_v<OObject, PointerToType>, CObjectPtrProperty, CPtrProperty>;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<std::is_pointer_v<T>>>
{
	using PointerToType = std::remove_pointer_t<T>;
	using PointerToPropType = typename TChooseProperty<PointerToType>::Type;
	using Type = std::conditional_t<std::is_base_of_v<OObject, PointerToType>, CObjectPtrProperty, CPtrProperty>;
};
template <typename T>
struct TChooseProperty<T, std::enable_if_t<TIsTArray<T>::Value>>
{
	using ElementType = typename TArrayTraits<T>::ElementType;
	using ElementPropType = typename TChooseProperty<ElementType>::Type;
	using Type = CArrayProperty;
};

template <typename PropertyType>
FORCEINLINE void SetStandardProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	assert(InController);
	if constexpr (std::is_reference_v<PropertyType> || std::is_pointer_v<PropertyType>)
	{
		SetStandardClassPtrProperty(OutProperty, InClassName, InController);
	}
	else
	{
		SetStandardClassProperty(OutProperty, InClassName, InController);
	}
}

template <>
FORCEINLINE void SetStandardProperty<Boolean>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCBoolProperty;
}
template <>
FORCEINLINE void SetStandardProperty<I8>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCI8Property;
}
template <>
FORCEINLINE void SetStandardProperty<I16>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCI16Property;
}
template <>
FORCEINLINE void SetStandardProperty<I32>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCI32Property;
}
template <>
FORCEINLINE void SetStandardProperty<I64>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCI64Property;
}
template <>
FORCEINLINE void SetStandardProperty<U8>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCU8Property;
}
template <>
FORCEINLINE void SetStandardProperty<U16>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCU16Property;
}
template <>
FORCEINLINE void SetStandardProperty<U32>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCU32Property;
}
template <>
FORCEINLINE void SetStandardProperty<U64>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCU64Property;
}
template <>
FORCEINLINE void SetStandardProperty<F32>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCF32Property;
}
template <>
FORCEINLINE void SetStandardProperty<F64>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCF64Property;
}
template <>
FORCEINLINE void SetStandardProperty<std::string>(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	*OutProperty = &StandardCStrProperty;
}

template <typename T>
struct TPropertyGetterTraitsBase
{
	using ClassType = void;
	using PropertyType = std::remove_pointer_t<T>;
};

template <typename C, typename T>
struct TPropertyGetterTraitsBase<T C::*>
{
	using ClassType = C;
	using PropertyType = T;
};

template <typename T>
struct TPropertyGetterTraits : public TPropertyGetterTraitsBase<T>
{
};

// 用于将函数参数类型转换到 TPropertyGetterTraitsBase<T> 可识别的全局指针类型
// 1. 如果是引用, 将其转换为指针
// 2. 再将其转换为此指向此类型的指针
template <typename T>
using TConvertToPropertyGetter = std::add_pointer_t<std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<std::remove_reference_t<T>>, T>>;

// 使用一个本地命名空唯一结构(UnqiueAnonymousType)保证唯一性
// 命名规则
// >> >> [1][2]____[3] << <<
// >> [1] 全局为G, 类为C, 函数为F
// >> [2] 属性为P, 类为C, 函数为F
// >> [3] 在所属空间的名称
// 通过模板创建一个唯一的静态属性
template <typename UnqiueAnonymousType, typename PropertyGetterType>
FORCEINLINE auto StaticCreateUniqueProperty(CMetadata* InOwner, const std::string& InName, CMetaProperty::OffsetSizeType InOffset, EPropertyFlags InPropertyFlags, const char* InStandardProperty, CController* InController)
{
	assert(InController);
	using PropertyType = TPropertyGetterTraits<PropertyGetterType>::PropertyType;
	using MetaPropertyType = typename TChooseProperty<PropertyType>::Type;
	static MetaPropertyType LProperty(InOwner, InName, InOffset, InPropertyFlags);
	if constexpr (TIsTArray<PropertyType>::Value)
	{
		using ElementType = typename TChooseProperty<PropertyType>::ElementType;
		using ElementPropType = typename TChooseProperty<PropertyType>::ElementPropType;
		SetStandardProperty<ElementType>(&(LProperty.ElementProp), InStandardProperty, InController);
	}
	else if constexpr (std::is_pointer_v<PropertyType> || std::is_reference_v<PropertyType>)
	{
		using PointerToType = typename TChooseProperty<PropertyType>::PointerToType;
		using PointerToPropType = typename TChooseProperty<PropertyType>::PointerToPropType;
		SetStandardProperty<PointerToType>(&(LProperty.PointerToProp), InStandardProperty, InController);
		LProperty.SetPropertyFlags(PF_ZeroConstructor | PF_NoDestructor);
	}
	else if constexpr (std::is_class_v<PropertyType>)
	{
		static_assert(std::is_default_constructible_v<PropertyType>);
		static_assert(std::is_copy_assignable_v<PropertyType>);
		static_assert(std::is_destructible_v<PropertyType>);
		if constexpr (std::is_trivially_default_constructible_v<PropertyType>)
		{
			LProperty.SetPropertyFlags(PF_ZeroConstructor);
		}
		if constexpr (std::is_trivially_destructible_v<PropertyType>)
		{
			LProperty.SetPropertyFlags(PF_ZeroConstructor);
		}
	}
	else if constexpr (std::is_arithmetic_v<PropertyType> || std::is_enum_v<PropertyType>)
	{
		LProperty.SetPropertyFlags(PF_ZeroConstructor | PF_NoDestructor);
	}
	else
	{
		static_assert(std::is_void_v<PropertyType>);
	}
	if (auto OwnerClass = InOwner->CastTo<CMetaClass>())
	{
		OwnerClass->InsertProperty(&LProperty);
	}
	else if (auto OwnerFunction = InOwner->CastTo<CMetaFunction>())
	{
		OwnerFunction->InsertProperty(&LProperty);
	}
	StaticSetConstAttrs<UnqiueAnonymousType>(&LProperty);
	InController->RegisterMetadata(&LProperty);
	return &LProperty;
}

// 使用一个本地命名空结构保证唯一性
// <<< [1][2]____[3] >>>
// [1] 全局为G, 类为C, 函数为F
// [2] 属性为P, 类为C, 函数为F
// [3] 在所属空间的名称
// 通过模板创建一个唯一的静态属性
template <typename UnqiueAnonymousType, typename OwnerType>
FORCEINLINE auto StaticCreateUniqueFunction(CMetadata* InOwner, const std::string& InName, asSFuncPtr FuncPtr, EFunctionFlags InFunctionFlags, CController* InController)
{
	static CMetaFunction LFunction(InOwner, InName, InFunctionFlags);
	if constexpr (std::is_void_v<OwnerType>)
	{
		LFunction.FuncPtr = FuncPtr;
	}
	else
	{
		LFunction.FuncPtr = FuncPtr;
	}
	if (auto OwnerClass = InOwner->CastTo<CMetaClass>())
	{
		OwnerClass->InsertFunction(&LFunction);
	}
	StaticSetConstAttrs<UnqiueAnonymousType>(&LFunction);
	InController->RegisterMetadata(&LFunction);
	return &LFunction;
}

RTCXX_NAMESPACE_END
