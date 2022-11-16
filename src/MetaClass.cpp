#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

IMPLEMENT_METADATA_CLASS(CMetaClass)

void CMetaClass::InsertFunction(CMetaFunction* InFunction)
{
	InFunction->FunctionLinkNext = FunctionLink;
	FunctionLink = InFunction;
}

RTCXX_NAMESPACE_END

