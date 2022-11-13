#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

asUINT CMetaClass::ConvertToScriptEngineTypeTraits()
{

	asDWORD flags = asOBJ_APP_CLASS;
	if (bHasConstructor)
		flags |= asOBJ_APP_CLASS_CONSTRUCTOR;
	if (bHasDestructor)
		flags |= asOBJ_APP_CLASS_DESTRUCTOR;
	if (bHasAssignmentOperator)
		flags |= asOBJ_APP_CLASS_ASSIGNMENT;
	if (bHasCopyConstructor)
		flags |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;
	return flags;
	// Unknown type traits
	return 0;
}

void CMetaClass::InsertProperty(CMetaProperty* InProperty)
{
	if (!PropertyLink)
	{
		PropertyLink = InProperty;
		return;
	}
	InProperty->PropertyLinkNext = PropertyLink;
	PropertyLink = InProperty;
}

void CMetaClass::InsertFunction(CMetaFunction* InFunction)
{
	if (!FunctionLink)
	{
		FunctionLink = InFunction;
		return;
	}
	InFunction->FunctionLinkNext = FunctionLink;
	FunctionLink = InFunction;
}

RTCXX_NAMESPACE_END

