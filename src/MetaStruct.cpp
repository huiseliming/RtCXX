#include "MetaStruct.h"
#include "MetaProperty.h"

RTCXX_NAMESPACE_BEGIN

IMPLEMENT_METADATA_CLASS(CMetaStruct)

void CMetaStruct::InsertProperty(CMetaProperty* InMetaProperty)
{
	if (!PropertyLink)
	{
		PropertyLink = InMetaProperty;
		return;
	}
	InMetaProperty->PropertyLinkNext = PropertyLink;
	PropertyLink = InMetaProperty;
}

RTCXX_NAMESPACE_END
