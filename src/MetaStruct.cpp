#include "MetaStruct.h"
#include "MetaProperty.h"

RTCXX_NAMESPACE_BEGIN

IMPLEMENT_METADATA_CLASS(CMetaStruct)

void CMetaStruct::InsertProperty(CMetaProperty* InMetaProperty)
{
	InMetaProperty->PropertyLinkNext = PropertyLink;
	PropertyLink = InMetaProperty;
}

void CMetaStruct::InsertSubStruct(CMetaStruct* InStruct)
{
	InStruct->SubStructLinkNext = SubStructLink;
	SubStructLink = InStruct;
}

RTCXX_NAMESPACE_END
