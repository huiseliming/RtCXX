#pragma once
#include "Metadata.h"
#include <typeindex>

RTCXX_NAMESPACE_BEGIN

class RTCXX_API CMetaStruct : public CMetadata
{
	DECLARE_METADATA_CLASS(CMetaStruct, CMetadata)
public:
	FORCEINLINE CMetaStruct(CMetadata* InOwner, const std::string& InName)
		: CMetadata(InOwner, InName, CMetaStruct::StaticMetadataClass())
	{
	}
	FORCEINLINE CMetaStruct(CMetadata* InOwner, const std::string& InName, CMetadataClass* InMetadataClass)
		: CMetadata(InOwner, InName, InMetadataClass)
	{
	}
public:
	void InsertProperty(CMetaProperty* InMetaProperty);
	CMetaProperty* PropertyLink = nullptr;

public:

};


RTCXX_NAMESPACE_END

