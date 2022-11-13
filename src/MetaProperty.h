#pragma once
#include "Metadata.h"
#include "Templates/BytesToSizeType.h"

#define DECLARE_PROPERTY(Class, SuperClass)                                   \
public:                                                                       \
	DECLARE_METADATA_STATIC_CAST_FLAG(Class)                                  \
	FORCEINLINE Class()                                                       \
		: SuperClass()                                                        \
	{                                                                         \
		CastFlags = EMetadataCastFlags(CastFlags | Class::StaticCastFlags()); \
	}                                                                         \
	FORCEINLINE Class(const std::string& InName, OffsetSizeType InOffset)     \
		: SuperClass(InName, InOffset)                                        \
	{                                                                         \
		CastFlags = EMetadataCastFlags(CastFlags | Class::StaticCastFlags()); \
	}

RTCXX_NAMESPACE_BEGIN

class RTCXX_API CMetaProperty : public CMetadata
{
public:
	using OffsetSizeType = TBytesToSizeType<sizeof(void*)>::Type;

public:
	DECLARE_METADATA_STATIC_CAST_FLAG(CMetaProperty)
	FORCEINLINE CMetaProperty()
		: CMetadata()
	{
		CastFlags = EMetadataCastFlags(CastFlags | CMetaProperty::StaticCastFlags());
	}
	FORCEINLINE CMetaProperty(const std::string& InName, OffsetSizeType InOffset)
		: CMetadata(InName)
		, Offset(InOffset)
	{
		CastFlags = EMetadataCastFlags(CastFlags | CMetaProperty::StaticCastFlags());
	}

public:
	std::string GetScriptDeclaration();

	auto GetValuePtr(void* InContainerPtr) const -> void* { return static_cast<U8*>(InContainerPtr) + Offset; }
	auto GetValuePtr(const void* InContainerPtr) const -> const void* { return static_cast<const U8*>(InContainerPtr) + Offset; }
	template <typename T>
	auto GetValuePtrAs(void* InContainerPtr) const -> T* { return static_cast<T*>(GetValuePtr(InContainerPtr)); }
	template <typename T>
	auto GetValueRefAs(void* InContainerPtr) const -> T& { return *static_cast<T*>(GetValuePtr(InContainerPtr)); }

	virtual std::string GetScriptTypeDecl()
	{
		return "void";
	}

	virtual void CopyValue(void* Dest, void const* Src) const { ; }
	virtual void MoveValue(void* Dest, void const* Src) const { ; }

	// 用于组成快速遍历的链
	CMetaProperty* PropertyLinkNext = nullptr;
	// CMetaClass* MetaClass = nullptr;
	OffsetSizeType Offset = 0;
	bool bIsStatic : 1;
	bool bIsReturn : 1;
	bool bIsPointer : 1;
	bool bIsReference : 1;
	bool bIsOut : 1;



protected:
	template <typename T>
	friend class TFunction;
};

class RTCXX_API CBoolProperty : public CMetaProperty
{
public:
	DECLARE_PROPERTY(CBoolProperty, CMetaProperty);
	std::string GetScriptTypeDecl() override;

public:
	bool GetBoolValue(void* InContainerPtr) { return GetValueRefAs<bool>(InContainerPtr); }
};

class RTCXX_API CNumericProperty : public CMetaProperty
{
public:
	DECLARE_PROPERTY(CNumericProperty, CMetaProperty)

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
	TNumericProperty()
		: CNumericProperty()
	{
	}
	TNumericProperty(const std::string& InName, OffsetSizeType InOffset)
		: CNumericProperty(InName, InOffset)
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
public:
	DECLARE_PROPERTY(CI8Property, TNumericProperty<I8>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI16Property : public TNumericProperty<I16>
{
public:
	DECLARE_PROPERTY(CI16Property, TNumericProperty<I16>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI32Property : public TNumericProperty<I32>
{
public:
	DECLARE_PROPERTY(CI32Property, TNumericProperty<I32>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CI64Property : public TNumericProperty<I64>
{
public:
	DECLARE_PROPERTY(CI64Property, TNumericProperty<I64>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU8Property : public TNumericProperty<U8>
{
public:
	DECLARE_PROPERTY(CU8Property, TNumericProperty<U8>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU16Property : public TNumericProperty<U16>
{
public:
	DECLARE_PROPERTY(CU16Property, TNumericProperty<U16>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU32Property : public TNumericProperty<U32>
{
public:
	DECLARE_PROPERTY(CU32Property, TNumericProperty<U32>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CU64Property : public TNumericProperty<U64>
{
public:
	DECLARE_PROPERTY(CU64Property, TNumericProperty<U64>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CF32Property : public TNumericProperty<F32>
{
public:
	DECLARE_PROPERTY(CF32Property, TNumericProperty<F32>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CF64Property : public TNumericProperty<F64>
{
public:
	DECLARE_PROPERTY(CF64Property, TNumericProperty<F64>)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CClassProperty : public CMetaProperty
{
public:
	DECLARE_PROPERTY(CClassProperty, CMetaProperty)
	std::string GetScriptTypeDecl() override;

	CMetaClass* MetaClass;
};

class RTCXX_API CStrProperty : public CClassProperty
{
public:
	DECLARE_PROPERTY(CStrProperty, CClassProperty)
	std::string GetScriptTypeDecl() override;
};

class RTCXX_API CObjectProperty : public CClassProperty
{
public:
	DECLARE_PROPERTY(CObjectProperty, CClassProperty)
	// std::string GetScriptTypeDecl() override { return "int8"; }
};

class RTCXX_API CPtrProperty : public CMetaProperty
{
public:
	DECLARE_PROPERTY(CPtrProperty, CMetaProperty)
	// std::string GetScriptTypeDecl() override { return "int8"; }

	CMetaProperty* PointerToProp;
};

class RTCXX_API CObjectPtrProperty : public CPtrProperty
{
public:
	DECLARE_PROPERTY(CObjectPtrProperty, CPtrProperty)
	// std::string GetScriptTypeDecl() override { return "int8"; }
};

class RTCXX_API CArrayProperty : public CMetaProperty
{
public:
	DECLARE_PROPERTY(CArrayProperty, CMetaProperty)
	// std::string GetScriptTypeDecl() override { return "int8"; }

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
//RTCXX_API extern CClassProperty StandardCClassProperty;
//RTCXX_API extern CObjectProperty StandardCObjectProperty;
//RTCXX_API extern CObjectPtrProperty StandardCObjectPtrProperty;
//RTCXX_API extern CArrayProperty StandardCArrayProperty;

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
//RTCXX_API extern CClassProperty StandardPtrCClassProperty;
//RTCXX_API extern CObjectProperty StandardPtrCObjectProperty;
//RTCXX_API extern CObjectPtrProperty StandardPtrCObjectPtrProperty;
//RTCXX_API extern CArrayProperty StandardPtrCArrayProperty;


 template <typename T, class Enabled = void>
 struct TSelectMetaProperty
{
	using Type = void;
};

 template <>
 struct TSelectMetaProperty<void>
{
	using Type = CMetaProperty;
};

 template <>
 struct TSelectMetaProperty<bool>
{
	using Type = CBoolProperty;
};

 template <>
 struct TSelectMetaProperty<I8>
{
	using Type = CI8Property;
};

 template <>
 struct TSelectMetaProperty<I16>
{
	using Type = CI16Property;
};

 template <>
 struct TSelectMetaProperty<I32>
{
	using Type = CI32Property;
};

 template <>
 struct TSelectMetaProperty<I64>
{
	using Type = CI64Property;
};

 template <>
 struct TSelectMetaProperty<U8>
{
	using Type = CU8Property;
};

 template <>
 struct TSelectMetaProperty<U16>
{
	using Type = CU16Property;
};

 template <>
 struct TSelectMetaProperty<U32>
{
	using Type = CU32Property;
};

 template <>
 struct TSelectMetaProperty<U64>
{
	using Type = CU64Property;
};

 template <>
 struct TSelectMetaProperty<F32>
{
	using Type = CF32Property;
};

 template <>
 struct TSelectMetaProperty<F64>
{
	using Type = CF64Property;
};

 template <>
 struct TSelectMetaProperty<std::string>
{
	using Type = CStrProperty;
};

 template <typename T>
 struct TSelectMetaProperty<T, std::enable_if_t<std::is_base_of_v<OObject, T>>>
{
	using Type = CObjectProperty;
};

 template <typename T>
 struct TSelectMetaProperty<T, std::enable_if_t<std::is_pointer_v<T> && std::is_base_of_v<OObject, std::remove_pointer_t<T>>>>
{
	using Type = CObjectPtrProperty;
};

 template <typename T>
 struct TSelectMetaProperty<T, std::enable_if_t<std::is_class_v<T>>>
{
	using Type = CClassProperty;
};

RTCXX_NAMESPACE_END