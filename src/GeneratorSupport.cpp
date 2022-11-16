#include "GeneratorSupport.h"


RTCXX_NAMESPACE_BEGIN

void SetInheritanceRelationship(CMetaClass* InMetaClass, const char* InBaseClassName, CController* InController)
{
	assert(InController);
	InController->GetClassByAfterRegisterClassCallback(
		InBaseClassName,
		[InMetaClass](CMetaClass* SuperClass) {
			InMetaClass->SuperStruct = SuperClass;
			SuperClass->InsertSubStruct(InMetaClass);
		});
}

void SetStandardClassPtrProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	InController->GetClassByAfterRegisterClassCallback(InClassName, [OutProperty](CMetaClass* InMetaClass) { *OutProperty = InMetaClass->StandardPtrProperty; });
}

void SetStandardClassProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	InController->GetClassByAfterRegisterClassCallback(InClassName, [OutProperty](CMetaClass* InMetaClass) { *OutProperty = InMetaClass->StandardProperty; });
}

RTCXX_NAMESPACE_END
