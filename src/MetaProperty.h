#pragma once
#include "Metadata.h"
#include "Templates/BytesToSizeType.h"

#define DECLARE_PROPERTY_CONSTRUCTOR(TClass, TSupperClass)                                                                                                        \
	FORCEINLINE TClass(CMetadata* InOwner, const std::string& InName, OffsetSizeType InOffsetOf, EPropertyFlags InPropertyFlags)                                  \
		: TSupperClass(InOwner, InName, InOffsetOf, InPropertyFlags, TClass::StaticMetadataClass())                                                               \
	{                                                                                                                                                             \
	}                                                                                                                                                             \
	FORCEINLINE TClass(CMetadata* InOwner, const std::string& InName, OffsetSizeType InOffsetOf, EPropertyFlags InPropertyFlags, CMetadataClass* InMetadataClass) \
		: TSupperClass(InOwner, InName, InOffsetOf, InPropertyFlags, InMetadataClass)                                                                             \
	{                                                                                                                                                             \
	}

RTCXX_NAMESPACE_BEGIN

enum EPropertyFlags : U64
{
	PF_None			   = 0x0000000000000000,
	PF_Param		   = 0x0000000000000001, // 标识此属性是函数的参数
	PF_OutParam		   = 0x0000000000000002, // 标识此属性是函数的输出参数
	PF_ReturnParam	   = 0x0000000000000004, // 标识此属性是C++函数的返回值
	PF_ReferenceParam  = 0x0000000000000008, // 标识此属性是函数通过应用传递的参数,其属性类型为CPtrProperty
	PF_ZeroConstructor = 0x0000000000000010, // 标识此属性不需要构造(可以直接进行内存拷贝)
	PF_NoDestructor	   = 0x0000000000000020, // 标识此属性不需要析构(可以直接进行内存拷贝)
	PF_Global		   = 0x8000000000000000, // 标识一个全局属性
	PF_AllFlags		   = 0xFFFFFFFFFFFFFFFF
};

DECL_ENUM_FLAGS(EPropertyFlags)

class RTCXX_API CMetaProperty : public CMetadata
{
	DECLARE_METADATA_CLASS(CMetaProperty, CMetadata)
public:
	using OffsetSizeType = TBytesToSizeType<sizeof(void*)>::Type;

public:
	FORCEINLINE CMetaProperty(CMetadata* InOwner, const std::string& InName, OffsetSizeType InOffsetOf, EPropertyFlags InPropertyFlags)
		: CMetadata(InOwner, InName, CMetaProperty::StaticMetadataClass())
		, Offset(InOffsetOf)
		, PropertyFlags(InPropertyFlags)
	{
	}
	FORCEINLINE CMetaProperty(CMetadata* InOwner, const std::string& InName, OffsetSizeType InOffsetOf, EPropertyFlags InPropertyFlags, CMetadataClass* InMetadataClass)
		: CMetadata(InOwner, InName, InMetadataClass)
		, Offset(InOffsetOf)
		, PropertyFlags(InPropertyFlags)
	{
	}
public:
	//同级属性链
	CMetaProperty* PropertyLinkNext = nullptr;

public:
	EPropertyFlags PropertyFlags;
	FORCEINLINE EPropertyFlags GetPropertyFlags() { return PropertyFlags; }
	FORCEINLINE void SetPropertyFlags(EPropertyFlags NewFlags) { PropertyFlags |= NewFlags; }
	FORCEINLINE void ClearPropertyFlags(EPropertyFlags NewFlags) { PropertyFlags &= ~NewFlags; }
	FORCEINLINE bool HasAnyPropertyFlags(EPropertyFlags FlagsToCheck) const { return (PropertyFlags & FlagsToCheck) != 0; }
	FORCEINLINE bool HasAllPropertyFlags(EPropertyFlags FlagsToCheck) const { return ((PropertyFlags & FlagsToCheck) == FlagsToCheck); }

public:
	std::string GetScriptDeclaration();
	// FORCEINLINE bool IsPointer() { return GetCastFlags() & MCF_CPtrProperty; }
	FORCEINLINE auto GetValuePtr(void* InContainerPtr) const -> void* { return static_cast<U8*>(InContainerPtr) + Offset; }
	FORCEINLINE auto GetValuePtr(const void* InContainerPtr) const -> const void* { return static_cast<const U8*>(InContainerPtr) + Offset; }
	template <typename T>
	FORCEINLINE auto GetValuePtrAs(void* InContainerPtr) const -> T* { return static_cast<T*>(GetValuePtr(InContainerPtr)); }
	template <typename T>
	FORCEINLINE auto GetValueRefAs(void* InContainerPtr) const -> T& { return *static_cast<T*>(GetValuePtr(InContainerPtr)); }

	virtual std::string GetScriptTypeDecl() { return "void"; }
	virtual void CopyValue(void* Dest, void const* Src) const { ; }
	virtual void MoveValue(void* Dest, void const* Src) const { ; }

	// CMetaClass* MetaClass = nullptr;
	OffsetSizeType Offset = 0;

	bool bIsStatic	  : 1;
	bool bIsReturn	  : 1;
	bool bIsPointer	  : 1;
	bool bIsReference : 1;
	bool bIsOut		  : 1;

protected:
	template <typename T>
	friend class TFunction;
};

class RTCXX_API CBoolProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CBoolProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CBoolProperty, CMetaProperty)
public:
	std::string GetScriptTypeDecl() override;

public:
	bool GetBoolValue(void* InContainerPtr) { return GetValueRefAs<bool>(InContainerPtr); }
};

class RTCXX_API CNumericProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CNumericProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CNumericProperty, CMetaProperty)
public:
public:
	virtual void SetI64Value(void* InContainerPtr, I64 InValue) { assert(0); }
	virtual void SetU64Value(void* InContainerPtr, U64 InValue) { assert(0); }
	virtual void SetF64Value(void* InContainerPtr, F64 InValue) { assert(0); }
	virtual void SetNumericValueFromString(void* InContainerPtr, const std::string& InValue) { assert(0); }

	virtual I64 GetI64Value(void* InContainerPtr)
	{
		assert(0);
		return 0;
	}
	virtual U64 GetU64Value(void* InContainerPtr)
	{
		assert(0);
		return 0;
	}
	virtual F64 GetF64Value(void* InContainerPtr)
	{
		assert(0);
		return 0.f;
	}
	virtual std::string GetNumericValueToString(void* InContainerPtr)
	{
		assert(0);
		return "";
	}
};

template <typename T>
class TNumericProperty : public CNumericProperty
{
public:
	FORCEINLINE TNumericProperty(CMetadata* InOwner, const std::string& InName, OffsetSizeType InOffsetOf, EPropertyFlags InPropertyFlags, CMetadataClass* InMetadataClass)
		: CNumericProperty(InOwner, InName, InOffsetOf, InPropertyFlags, InMetadataClass)
	{
	}

	I64 GetI64Value(void* InContainerPtr) override { return static_cast<T>(GetValueRefAs<T>(InContainerPtr)); }
	U64 GetU64Value(void* InContainerPtr) override { return static_cast<T>(GetValueRefAs<T>(InContainerPtr)); }
	F64 GetF64Value(void* InContainerPtr) override { return static_cast<T>(GetValueRefAs<T>(InContainerPtr)); }
	std::string GetNumericValueToString(void* InContainerPtr) override { return std::to_string(GetValueRefAs<T>(InContainerPtr)); }
	void SetI64Value(void* InContainerPtr, I64 InValue) override { GetValueRefAs<T>(InContainerPtr) = static_cast<T>(InValue); }
	void SetU64Value(void* InContainerPtr, U64 InValue) override { GetValueRefAs<T>(InContainerPtr) = static_cast<T>(InValue); }
	void SetF64Value(void* InContainerPtr, F64 InValue) override { GetValueRefAs<T>(InContainerPtr) = static_cast<T>(InValue); }
	void SetNumericValueFromString(void* InContainerPtr, const std::string& InValue) override
	{
		if constexpr (std::is_floating_point_v<T>)
			GetValueRefAs<T>(InContainerPtr) = atof(InValue.c_str());
		else if constexpr (std::is_unsigned_v<T>)
			GetValueRefAs<T>(InContainerPtr) = strtoull(InValue.c_str(), nullptr, 10);
		else if constexpr (std::is_signed_v<T>)
			GetValueRefAs<T>(InContainerPtr) = atoll(InValue.c_str());
		else
			assert(0);
	}
};

class RTCXX_API CI8Property : public TNumericProperty<I8>
{
	DECLARE_METADATA_CLASS(CI8Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CI8Property, TNumericProperty<I8>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI16Property : public TNumericProperty<I16>
{
	DECLARE_METADATA_CLASS(CI16Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CI16Property, TNumericProperty<I16>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI32Property : public TNumericProperty<I32>
{
	DECLARE_METADATA_CLASS(CI32Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CI32Property, TNumericProperty<I32>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI64Property : public TNumericProperty<I64>
{
	DECLARE_METADATA_CLASS(CI64Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CI64Property, TNumericProperty<I64>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU8Property : public TNumericProperty<U8>
{
	DECLARE_METADATA_CLASS(CU8Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CU8Property, TNumericProperty<U8>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU16Property : public TNumericProperty<U16>
{
	DECLARE_METADATA_CLASS(CU16Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CU16Property, TNumericProperty<U16>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU32Property : public TNumericProperty<U32>
{
	DECLARE_METADATA_CLASS(CU32Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CU32Property, TNumericProperty<U32>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU64Property : public TNumericProperty<U64>
{
	DECLARE_METADATA_CLASS(CU64Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CU64Property, TNumericProperty<U64>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CF32Property : public TNumericProperty<F32>
{
	DECLARE_METADATA_CLASS(CF32Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CF32Property, TNumericProperty<F32>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CF64Property : public TNumericProperty<F64>
{
	DECLARE_METADATA_CLASS(CF64Property, CNumericProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CF64Property, TNumericProperty<F64>)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CStrProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CStrProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CStrProperty, CMetaProperty)
public:
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CClassProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CClassProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CClassProperty, CMetaProperty)
public:
	std::string GetScriptTypeDecl() override;

	CMetaClass* MetaClass;
};

class RTCXX_API CPtrProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CPtrProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CPtrProperty, CMetaProperty)
public:
	std::string GetScriptTypeDecl() override { 
		std::string PointerToScriptTypeDecl = PointerToProp->GetScriptTypeDecl();
		return PointerToScriptTypeDecl + "&";
	}

	CMetaProperty* PointerToProp;
};

class RTCXX_API CObjectPtrProperty : public CPtrProperty
{
	DECLARE_METADATA_CLASS(CObjectPtrProperty, CPtrProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CObjectPtrProperty, CPtrProperty)
public:
};

class RTCXX_API CArrayProperty : public CMetaProperty
{
	DECLARE_METADATA_CLASS(CArrayProperty, CMetaProperty)
	DECLARE_PROPERTY_CONSTRUCTOR(CArrayProperty, CMetaProperty)
public:
	CMetaProperty* ElementProp;
};


RTCXX_API extern CBoolProperty StandardCBoolProperty;
RTCXX_API extern CNumericProperty StandardCNumericProperty;
RTCXX_API extern CI8Property StandardCI8Property;
RTCXX_API extern CI16Property StandardCI16Property;
RTCXX_API extern CI32Property StandardCI32Property;
RTCXX_API extern CI64Property StandardCI64Property;
RTCXX_API extern CU8Property StandardCU8Property;
RTCXX_API extern CU16Property StandardCU16Property;
RTCXX_API extern CU32Property StandardCU32Property;
RTCXX_API extern CU64Property StandardCU64Property;
RTCXX_API extern CF32Property StandardCF32Property;
RTCXX_API extern CF64Property StandardCF64Property;
RTCXX_API extern CStrProperty StandardCStrProperty;
// RTCXX_API extern CClassProperty StandardCClassProperty;
// RTCXX_API extern CObjectProperty StandardCObjectProperty;
// RTCXX_API extern CObjectPtrProperty StandardCObjectPtrProperty;
// RTCXX_API extern CArrayProperty StandardCArrayProperty;

RTCXX_API extern CPtrProperty StandardPtrCBoolProperty;
RTCXX_API extern CPtrProperty StandardPtrCNumericProperty;
RTCXX_API extern CPtrProperty StandardPtrCI8Property;
RTCXX_API extern CPtrProperty StandardPtrCI16Property;
RTCXX_API extern CPtrProperty StandardPtrCI32Property;
RTCXX_API extern CPtrProperty StandardPtrCI64Property;
RTCXX_API extern CPtrProperty StandardPtrCU8Property;
RTCXX_API extern CPtrProperty StandardPtrCU16Property;
RTCXX_API extern CPtrProperty StandardPtrCU32Property;
RTCXX_API extern CPtrProperty StandardPtrCU64Property;
RTCXX_API extern CPtrProperty StandardPtrCF32Property;
RTCXX_API extern CPtrProperty StandardPtrCF64Property;
RTCXX_API extern CPtrProperty StandardPtrCStrProperty;
// RTCXX_API extern CClassProperty StandardPtrCClassProperty;
// RTCXX_API extern CObjectProperty StandardPtrCObjectProperty;
// RTCXX_API extern CObjectPtrProperty StandardPtrCObjectPtrProperty;
// RTCXX_API extern CArrayProperty StandardPtrCArrayProperty;

//template <typename T, class Enabled = void>
//struct TMetaProperty
//{
//	using Type = void;
//};
//
//template <>
//struct TMetaProperty<void>
//{
//	using Type = CMetaProperty;
//};
//
//template <>
//struct TMetaProperty<bool>
//{
//	using Type = CBoolProperty;
//};
//
//template <>
//struct TMetaProperty<I8>
//{
//	using Type = CI8Property;
//};
//
//template <>
//struct TMetaProperty<I16>
//{
//	using Type = CI16Property;
//};
//
//template <>
//struct TMetaProperty<I32>
//{
//	using Type = CI32Property;
//};
//
//template <>
//struct TMetaProperty<I64>
//{
//	using Type = CI64Property;
//};
//
//template <>
//struct TMetaProperty<U8>
//{
//	using Type = CU8Property;
//};
//
//template <>
//struct TMetaProperty<U16>
//{
//	using Type = CU16Property;
//};
//
//template <>
//struct TMetaProperty<U32>
//{
//	using Type = CU32Property;
//};
//
//template <>
//struct TMetaProperty<U64>
//{
//	using Type = CU64Property;
//};
//
//template <>
//struct TMetaProperty<F32>
//{
//	using Type = CF32Property;
//};
//
//template <>
//struct TMetaProperty<F64>
//{
//	using Type = CF64Property;
//};
//
//template <>
//struct TMetaProperty<std::string>
//{
//	using Type = CStrProperty;
//};
//
//// template <typename T>
//// struct TMetaProperty<T, std::enable_if_t<std::is_base_of_v<OObject, T>>>
////{
////	using Type = CObjectProperty;
//// };
//
//template <typename T>
//struct TMetaProperty<T, std::enable_if_t<std::is_pointer_v<T> && std::is_base_of_v<OObject, std::remove_pointer_t<T>>>>
//{
//	using Type = CObjectPtrProperty;
//};
//
//template <typename T>
//struct TMetaProperty<T, std::enable_if_t<std::is_class_v<T>>>
//{
//	using Type = CClassProperty;
//};

RTCXX_NAMESPACE_END