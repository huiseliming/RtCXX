#include "RtCXX.h"
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>

RTCXX_NAMESPACE_BEGIN

CController* GController = GetControllerPtr();
CController* GetControllerPtr()
{
	static CController Controller;
	return &Controller;
}

CController::CController()
{
	ScriptEngine = asCreateScriptEngine();
	RegisterStdString(ScriptEngine);
	int r;
	// https://www.angelcode.com/angelscript/sdk/docs/manual/doc_global_typedef.html
	// r = ScriptEngine->RegisterTypedef("Void"   , "void"); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("I8"     , "int8"  ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("I16"    , "int16" ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("I32"    , "int"   ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("I64"    , "int64" ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("U8"     , "uint8" ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("U16"    , "uint16"); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("U32"    , "uint"  ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("U64"    , "uint64"); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("F32"    , "float" ); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("F64"    , "double"); assert(r >= 0);
	r = ScriptEngine->RegisterTypedef("Boolean", "bool"  ); assert(r >= 0);
	// r = ScriptEngine->RegisterTypedef("String" , "string"); assert(r >= 0);
} 

CController::~CController()
{
	ScriptEngine->ShutDownAndRelease();
}

bool CController::RegisterMetadata(CMetadata* InMetadata)
{
	auto InsertResult = MetadataNameMap.insert(std::make_pair(InMetadata->GetFullName(), InMetadata));
	if (InsertResult.second)
	{
		if (auto MetaClass = InMetadata->CastTo<CMetaClass>())
		{
			auto InsertResult2 = TypeIndexMap.insert(std::make_pair(MetaClass->TypeIndex, MetaClass));
			if (InsertResult2.second)
			{
				MetaClass->Id = Metadatas.size();
				Metadatas.push_back(MetaClass);
				ClassIndices.push_back(MetaClass->Id);
				auto AfterRegisterClassCallbackListMapIterator = AfterRegisterClassCallbackListMap.find(MetaClass->TypeIndex);
				if (AfterRegisterClassCallbackListMapIterator != AfterRegisterClassCallbackListMap.end())
				{
					for (auto& AfterRegisterClassCallback : AfterRegisterClassCallbackListMapIterator->second)
					{
						AfterRegisterClassCallback(MetaClass);
					}
					AfterRegisterClassCallbackListMap.erase(AfterRegisterClassCallbackListMapIterator);
				}

				auto AfterRegisterClassNameCallbackListMapIterator = AfterRegisterClassNameCallbackListMap.find(InMetadata->GetFullName());
				if (AfterRegisterClassNameCallbackListMapIterator != AfterRegisterClassNameCallbackListMap.end())
				{
					for (auto& AfterRegisterClassCallback : AfterRegisterClassNameCallbackListMapIterator->second)
					{
						AfterRegisterClassCallback(MetaClass);
					}
					AfterRegisterClassNameCallbackListMap.erase(AfterRegisterClassNameCallbackListMapIterator);
				}
			}
			else
			{
				MetadataNameMap.erase(InMetadata->Name);
				THROW_STD_EXCEPTION();
				return false;
			}
		}
		else if (auto MetaFunction = InMetadata->CastTo<CMetaFunction>())
		{
			MetaFunction->Id = Metadatas.size();
			Metadatas.push_back(MetaFunction);
		}
		else if (auto MetaProperty = InMetadata->CastTo<CMetaProperty>())
		{
			MetaProperty->Id = Metadatas.size();
			Metadatas.push_back(MetaProperty);
		}
		else
		{
			THROW_STD_EXCEPTION();
		}
		return true;
	}
	THROW_STD_EXCEPTION();
	return false;
}

void CController::GetClassByAfterRegisterClassCallback(std::type_index InTypeIndex, std::function<void(CMetaClass*)>&& InAfterRegisterClassCallback)
{
	auto It = TypeIndexMap.find(InTypeIndex);
	if (It != TypeIndexMap.end())
	{
		return InAfterRegisterClassCallback(It->second);
	}
	AfterRegisterClassCallbackListMap[InTypeIndex].push_back(std::move(InAfterRegisterClassCallback));
}

void CController::GetClassByAfterRegisterClassCallback(const std::string& InTypeName, std::function<void(CMetaClass*)>&& InAfterRegisterClassCallback)
{
	auto It = MetadataNameMap.find(InTypeName);
	if (It != MetadataNameMap.end())
	{
		return InAfterRegisterClassCallback(It->second->CastTo<CMetaClass>());
	}
	AfterRegisterClassNameCallbackListMap[InTypeName].push_back(std::move(InAfterRegisterClassCallback));
}

void CController::BuildInheritedData()
{	
	// CastRanges [first, second)
	// first = SelfCastIndex
	// second = LastCanCast + 1;
	struct CLoopper
	{
		void operator()(CMetaClass* CurrentClass)
		{
			CurrentClass->CastRanges.first = CastIndexCounter++;
			auto DerivedClassLinkNext = CurrentClass->GetSubClassLink();
			while (DerivedClassLinkNext)
			{
				(*this)(DerivedClassLinkNext);
				DerivedClassLinkNext = DerivedClassLinkNext->GetSubClassLinkNext();
			}
			CurrentClass->CastRanges.second = CastIndexCounter;
		}
		u32 CastIndexCounter = 0;
	} Loopper;
	for (size_t i = 0; i < ClassIndices.size(); i++)
	{
		auto MetaClass = static_cast<CMetaClass*>(Metadatas[ClassIndices[i]]);
		if (!MetaClass->GetSuperClass())
		{
			Loopper(MetaClass);
		}
	}
}

void CController::RegisterToScriptEngine()
{
	for (size_t i = 0; i < ClassIndices.size(); i++)
	{
		auto MetaClass = static_cast<CMetaClass*>(Metadatas[ClassIndices[i]]);
		//if (!MetaClass->HasTypeTraits(U32(ETypeTraits::TT_IsArithmeticBits) | U32(ETypeTraits::TT_Void) | U32(ETypeTraits::TT_IsString)))
		{
			MetaClass->RegisterToScriptEngine(ScriptEngine);
		}
	}
}

void CController::Init()
{
	if (!AfterRegisterClassCallbackListMap.empty())
		THROW_STD_EXCEPTION();
	if (!AfterRegisterClassNameCallbackListMap.empty())
		THROW_STD_EXCEPTION();
	BuildInheritedData();
	RegisterToScriptEngine();
}

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
