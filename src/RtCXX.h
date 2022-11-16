#pragma once
#include <functional>
#include "Fwd.h"
#include "MetaClass.h"
#include "angelscript.h"
#include "Cast.h"

RTCXX_NAMESPACE_BEGIN

RTCXX_API extern CController* GController;
RTCXX_API CController* GetControllerPtr();
FORCEINLINE CController& GetControllerRef()
{
	return *GetControllerPtr();
}

class RTCXX_API CController
{
	CController();

public:
	~CController();

	bool RegisterMetadata(CMetadata* InMetadata);

	template <typename T>
	T* FindMetadataAs(const std::string& InName)
	{
		auto find_result = MetadataNameMap.find(InName);
		if (find_result != MetadataNameMap.end())
		{
			return find_result->second->CastTo<T>();
		}
		return nullptr;
	}

	template <typename T>
	T* GetMetadataAs(I32 InMetadataIndex)
	{
		return Metadatas[InMetadataIndex]->CastTo<T>();
	}

	void GetClassByAfterRegisterClassCallback(std::type_index InTypeIndex, std::function<void(CMetaClass*)>&& InAfterRegisterClassCallback);
	void GetClassByAfterRegisterClassCallback(const std::string& InTypeName, std::function<void(CMetaClass*)>&& InAfterRegisterClassCallback);

	void BuildInheritedData();
	void RegisterToScriptEngine();
	void Init();

public:
	std::vector<CMetadata*> Metadatas;

	// type
	std::vector<i32> ClassIndices;
	std::unordered_map<std::type_index, CMetaClass*> TypeIndexMap;

	std::unordered_map<std::type_index, std::list<std::function<void(CMetaClass*)>>> AfterRegisterClassCallbackListMap;

	std::unordered_map<std::string, std::list<std::function<void(CMetaClass*)>>> AfterRegisterClassNameCallbackListMap;
	// std::list<std::function<void()>> InitFnList;
	// std::list<std::unique_ptr<CMetadata>> LifeCycleList;

	std::unordered_map<std::string, CMetadata*> MetadataNameMap;

	asIScriptEngine* ScriptEngine;

private:
	friend RTCXX_API CController* GetControllerPtr();
};

RTCXX_NAMESPACE_END
