#pragma once
#include "Metadata.h"
#include "MetaProperty.h"
#include <angelscript.h>

RTCXX_NAMESPACE_BEGIN

enum EFunctionFlags : U64
{
	FF_None		= 0x0000000000000000,
	FF_Global	= 0x8000000000000000, // 标识一个全局函数
	FF_AllFlags = 0xFFFFFFFFFFFFFFFF
};
DECL_ENUM_FLAGS(EFunctionFlags)

class RTCXX_API CMetaFunction : public CMetadata
{
	DECLARE_METADATA_CLASS(CMetaFunction, CMetadata)
public:
	FORCEINLINE CMetaFunction(CMetadata* InOwner, const std::string& InName, EFunctionFlags InFunctionFlags)
		: CMetadata(InOwner, InName, CMetaProperty::StaticMetadataClass())
		, FunctionFlags(InFunctionFlags)
	{
	}
	FORCEINLINE CMetaFunction(CMetadata* InOwner, const std::string& InName, EFunctionFlags InFunctionFlags, CMetadataClass* InMetadataClass)
		: CMetadata(InOwner, InName, InMetadataClass)
		, FunctionFlags(InFunctionFlags)
	{
	}

public:
	EFunctionFlags FunctionFlags;
	FORCEINLINE EFunctionFlags GetFunctionFlags() { return FunctionFlags; }
	FORCEINLINE void SetFunctionFlags(EFunctionFlags NewFlags) { FunctionFlags |= NewFlags; }
	FORCEINLINE void ClearFunctionFlags(EFunctionFlags NewFlags) { FunctionFlags &= ~NewFlags; }
	FORCEINLINE bool HasAnyFunctionFlags(EFunctionFlags FlagsToCheck) const { return (FunctionFlags & FlagsToCheck) != 0; }
	FORCEINLINE bool HasAllFunctionFlags(EFunctionFlags FlagsToCheck) const { return ((FunctionFlags & FlagsToCheck) == FlagsToCheck); }

public:
	std::string GetScriptDeclaration();
	void InsertProperty(CMetaProperty* InMetaProperty);

public:
	// 用于组成快速遍历的链
	CMetaFunction* FunctionLinkNext = nullptr;
	CMetaProperty* PropertyLink		= nullptr;
	asSFuncPtr FuncPtr;
	bool bIsStatic : 1;

protected:
};

RTCXX_NAMESPACE_END
