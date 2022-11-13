#pragma once
#include "MetaFunction.h"
#include <typeindex>
RTCXX_NAMESPACE_BEGIN  

class RTCXX_API CMetaClass : public CMetadata
{
public: 
	DECLARE_METADATA(CMetaClass, CMetadata)
	 
	virtual void RegisterToScriptEngine(asIScriptEngine* ScriptEngine) {}
public:   

	asUINT ConvertToScriptEngineTypeTraits();
	void InsertProperty(CMetaProperty* InProperty);
	void InsertFunction(CMetaFunction* InFunction);
	
	auto New() -> void* { void* instance_ptr = std::malloc(SizeOf); Constructor(instance_ptr); return instance_ptr; }
	auto CopyNew(void* other_ptr) -> void* { void* instance_ptr = std::malloc(SizeOf); CopyConstructor(instance_ptr, other_ptr); return instance_ptr; }
	auto MoveNew(void* other_ptr) -> void* { void* instance_ptr = std::malloc(SizeOf); MoveConstructor(instance_ptr, other_ptr); return instance_ptr; }
	auto Delete(void* instance_ptr) -> void { Destructor(instance_ptr); std::free(instance_ptr); }
	
	static void DefaultCopyAssign(void* instance_ptr, const void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultMoveAssign(void* instance_ptr, void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultConstructor(void* instance_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultCopyConstructor(void* instance_ptr, const void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultMoveConstructor(void* instance_ptr, void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultDestructor(void* instance_ptr) { THROW_STD_EXCEPTION(); }

	void (*CopyAssign)(void* instance_ptr, const void* other_ptr)      = &DefaultCopyAssign;
	void (*MoveAssign)(void* instance_ptr, void* other_ptr)            = &DefaultMoveAssign;
	void (*Constructor)(void* instance_ptr)                            = &DefaultConstructor;
	void (*CopyConstructor)(void* instance_ptr, const void* other_ptr) = &DefaultCopyConstructor;
	void (*MoveConstructor)(void* instance_ptr, void* other_ptr)       = &DefaultMoveConstructor;
	void (*Destructor)(void* instance_ptr)                             = &DefaultDestructor;

	CMetaProperty* StandardProperty;
	CMetaProperty* StandardPtrProperty;

	// bool IsExtendsFrom(CMetaClass* target_extends_class)
	//{
	//	auto extends_type = ExtendsClass;
	//	while (extends_type != nullptr)
	//	{
	//		if (extends_type == target_extends_class)
	//			return true;
	//		extends_type = extends_type->ExtendsClass;
	//	}
	//	return false;
	// }
	 
	std::type_index TypeIndex = typeid(FNull);
	I32 SizeOf;
	CMetaClass* ExtendsClass;
	std::vector<CMetaClass*> DerivedClasses;
	CMetaProperty* PropertyLink;
	CMetaFunction* FunctionLink;
	std::pair<u32, u32> CastRanges;
	bool bHasConstructor : 1;
	bool bHasDestructor : 1;
	bool bHasAssignmentOperator : 1;
	bool bHasCopyConstructor : 1;

};

RTCXX_NAMESPACE_END