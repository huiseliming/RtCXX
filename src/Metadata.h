#pragma once
#include "Fwd.h"
#include <any>
#include <string>
#include <variant>
#include <unordered_map>

RTCXX_NAMESPACE_BEGIN

enum EMetadataCastFlags : U32
{
	MCF_None			   = 0x00000000,
	MCF_CMetadata		   = 0x00000001,
	MCF_CMetaStruct        = 0x00000002,
	MCF_CMetaClass		   = 0x00000004,
	MCF_CMetaFunction	   = 0x00000008,
	MCF_CMetaProperty	   = 0x00000010,
	MCF_CBoolProperty	   = 0x00000020,
	MCF_CNumericProperty   = 0x00000040,
	MCF_CI8Property		   = 0x00000080,
	MCF_CI16Property	   = 0x00000100,
	MCF_CI32Property	   = 0x00000200,
	MCF_CI64Property	   = 0x00000400,
	MCF_CU8Property		   = 0x00000800,
	MCF_CU16Property	   = 0x00001000,
	MCF_CU32Property	   = 0x00002000,
	MCF_CU64Property	   = 0x00004000,
	MCF_CF32Property	   = 0x00008000,
	MCF_CF64Property	   = 0x00010000,
	MCF_CStrProperty	   = 0x00020000,
	MCF_CClassProperty	   = 0x00040000,
	MCF_CObjectProperty	   = 0x00080000,
	MCF_CPtrProperty	   = 0x00100000,
	MCF_CObjectPtrProperty = 0x00200000,
	MCF_CArrayProperty	   = 0x00400000,
};
DECL_ENUM_FLAGS(EMetadataCastFlags);

class RTCXX_API CMetadataClass
{
public:
	CMetadataClass(EMetadataCastFlags InCastFlag, EMetadataCastFlags InCastFlags)
		: MetadataCastFlag(InCastFlag)
		, MetadataCastFlags(InCastFlags)
	{
	}
	EMetadataCastFlags MetadataCastFlag;
	EMetadataCastFlags MetadataCastFlags;
};

template <typename T>
constexpr EMetadataCastFlags SuperClassStaticCastFlags()
{
	return T::StaticCastFlags();
}

template <>
constexpr EMetadataCastFlags SuperClassStaticCastFlags<void>()
{
	return MCF_None;
}

#define DECLARE_METADATA_CLASS(TClass, TSupperClass)                                                  \
public:                                                                                               \
	constexpr static EMetadataCastFlags StaticCastFlag() { return EMetadataCastFlags(MCF_##TClass); } \
	constexpr static EMetadataCastFlags StaticCastFlags()                                             \
	{                                                                                                 \
		return TClass::StaticCastFlag() | SuperClassStaticCastFlags<TSupperClass>();                  \
	}                                                                                                 \
	static CMetadataClass* GVar_StaticMetadataClass;                                                  \
	static CMetadataClass* StaticMetadataClass();

#define IMPLEMENT_METADATA_CLASS(TClass)                                                           \
	CMetadataClass* TClass::GVar_StaticMetadataClass = TClass::StaticMetadataClass();              \
	CMetadataClass* TClass::StaticMetadataClass()                                                  \
	{                                                                                              \
		static CMetadataClass LMetadataClass(TClass::StaticCastFlag(), TClass::StaticCastFlags()); \
		return &LMetadataClass;                                                                    \
	}

#define DECLARE_METADATA_CONSTRUCT(TClass, TSupperClass)                                               \
	FORCEINLINE TClass(CMetadata* InOwner, const std::string& InName)                                  \
		: TSupperClass(InOwner, InName, TClass::StaticMetadataClass())                                 \
	{                                                                                                  \
	}                                                                                                  \
	FORCEINLINE TClass(CMetadata* InOwner, const std::string& InName, CMetadataClass* InMetadataClass) \
		: TSupperClass(InOwner, InName, InMetadataClass)                                               \
	{                                                                                                  \
	}

class RTCXX_API CMetadata
{
public:
	DECLARE_METADATA_CLASS(CMetadata, void)
	template <typename T>
	T* CastTo()
	{
		if (MetadataClass->MetadataCastFlags & T::StaticMetadataClass()->MetadataCastFlag)
			return static_cast<T*>(this);
		return nullptr;
	}

public:
	FORCEINLINE CMetadata(CMetadata* InOwner, const std::string& InName)
		: Owner(InOwner)
		, Name(InName)
		, MetadataClass(StaticMetadataClass())
	{
	}
	FORCEINLINE CMetadata(CMetadata* InOwner, const std::string& InName, CMetadataClass* InMetadataClass)
		: Owner(InOwner)
		, Name(InName)
		, MetadataClass(InMetadataClass)
	{
	}
	CMetadata(const CMetadata&) = delete;
	CMetadata(CMetadata&&)		= delete;
	CMetadata& operator=(const CMetadata&) = delete;
	CMetadata& operator=(CMetadata&&) = delete;
	virtual ~CMetadata() {}

	FORCEINLINE std::string GetFullName()
	{
		std::string FullName = Name;
		CMetadata* Owner	 = GetOwner();
		while (Owner)
		{
			FullName = Owner->GetName() + "::" + FullName;
			Owner	 = Owner->GetOwner();
		}
		return FullName;
	}

	FORCEINLINE I32 GetId() const { return Id; }
	FORCEINLINE const std::string& GetName() const { return Name; }
	FORCEINLINE CMetadata* GetOwner() const { return Owner; }
	//U64 GetCastFlags() { return CastFlags; }

	FORCEINLINE bool HasAttr(const std::string& InName, const std::any** OutAttrAny = nullptr) const
	{
		for (auto& Attribute : Attributes)
		{
			if (InName == Attribute.first)
			{
				if (OutAttrAny)
				{
					*OutAttrAny = &Attribute.second;
				}
			}
			return true;
		}
		return false;
	}

	FORCEINLINE std::any GetAttr(const std::string& InName) const
	{
		const std::any* AttrAnyPtr = nullptr;
		if (HasAttr(InName, &AttrAnyPtr))
		{
			return *AttrAnyPtr;
		}
		return std::any();
	}

	template<typename T>
	FORCEINLINE T GetAttrAs(const std::string& InName) const
	{
		const std::any* AttrAnyPtr = nullptr;
		if (HasAttr(InName, &AttrAnyPtr))
		{
			static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string>);
			if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, const char*>)
			{
				if (AttrAnyPtr->type() == typeid(T)) return std::any_cast<T>(*AttrAnyPtr);
			}
			else
			{
				if (AttrAnyPtr->type() == typeid(const char*)) return std::any_cast<const char*>(*AttrAnyPtr);
				if (AttrAnyPtr->type() == typeid(std::string)) return std::any_cast<std::string>(*AttrAnyPtr);
			}
		}
		return T();
	}

	FORCEINLINE void SetAttr(const std::string& InName, const std::any& InValue)
	{
		const std::any* AttrAnyPtr = nullptr;
		if (HasAttr(InName, &AttrAnyPtr))
			*const_cast<std::any*>(AttrAnyPtr) = InValue;
		else
			Attributes.push_back(std::pair(InName, InValue));
	}

	FORCEINLINE void SetAttr(const std::string& InName, std::any&& InValue)
	{
		const std::any* AttrAnyPtr = nullptr;
		if (HasAttr(InName, &AttrAnyPtr))
			*const_cast<std::any*>(AttrAnyPtr) = std::move(InValue);
		else
			Attributes.push_back(std::pair(InName, std::move(InValue)));
	}

	template<typename T>
	FORCEINLINE void SetAttrValue(const std::string& InName, const T& InValue)
	{
		static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string>);
		SetAttr(InName, InValue);
	}

	FORCEINLINE void SetAttrValue(const std::string& InName, std::string&& InValue) { SetAttr(InName, std::move(InValue)); }

public:
	void SetName(const std::string& InName) { Name = InName; }
	void SetOwner(CMetadata* InOwner) { Owner = InOwner; }

protected:
	I32 Id;
	CMetadata* Owner;
	std::string Name;
	CMetadataClass* MetadataClass;
	//EMetadataCastFlags CastFlags;

	std::vector<std::pair<std::string, std::any>> Attributes;

private:
	friend class CController;

public:
	using ConstAttrValueType = std::variant<Boolean, I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, const char*>;
	using ConstAttrType = std::pair<const char*, ConstAttrValueType>;
};

RTCXX_NAMESPACE_END