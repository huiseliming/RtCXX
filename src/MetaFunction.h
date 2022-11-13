#pragma once
#include "Metadata.h"
#include "MetaProperty.h"
#include <angelscript.h>

RTCXX_NAMESPACE_BEGIN

class RTCXX_API CMetaFunction : public CMetadata
{
public:
	DECLARE_METADATA(CMetaFunction, CMetadata)

public:
	std::string GetScriptDeclaration();
	void InsertProperty(CMetaProperty* InMetaProperty);

	// 用于组成快速遍历的链
	CMetaFunction* FunctionLinkNext = nullptr;
	CMetaProperty* PropertyLink = nullptr;
	asSFuncPtr FuncPtr;
	bool bIsStatic : 1;

protected:
};

RTCXX_NAMESPACE_END
