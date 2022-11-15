#pragma once
#include <functional>
#include "Fwd.h"
#include "MetaClass.h"
#include "angelscript.h"
#include "Templates/FunctionTraits.h"
#include "Templates/Null.h"
#include "Cast.h"
#include "Templates/IsArray.h"

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

template <typename T>
class TClass : public CMetaClass
{
public:
	TClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags)
		: CMetaClass(InOwner, InName, InClassFlags)
	{
		TypeIndex = typeid(T);
		bHasConstructor = std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value;
		bHasDestructor = std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value;
		bHasAssignmentOperator = std::is_copy_assignable<T>::value && !std::is_trivially_copy_assignable<T>::value;
		bHasCopyConstructor = std::is_copy_constructible<T>::value && !std::is_trivially_copy_constructible<T>::value;
		
		SizeOf = sizeof(T);

		CopyAssign      = &CopyAssignImpl;
		MoveAssign      = &MoveAssignImpl;
		Constructor     = &ConstructorImpl;
		CopyConstructor = &CopyConstructorImpl;
		MoveConstructor = &MoveConstructorImpl;
		Destructor      = &DestructorImpl;

		static CClassProperty ClassProp(nullptr, "Standard" + Name, 0, PF_None);
		ClassProp.bIsPointer = false;
		ClassProp.MetaClass = this;
		StandardProperty = &ClassProp;
		if constexpr (std::is_base_of_v<OObject, T>)
		{
			static CObjectPtrProperty ObjPtrProp(nullptr, "StandardPtr" + Name, 0, PF_None);
			ObjPtrProp.bIsPointer = true;
			ObjPtrProp.PointerToProp = &ClassProp;
			StandardPtrProperty = &ObjPtrProp;
		}
		else
		{
			static CPtrProperty ClassPtrProp(nullptr, "StandardPtr" + Name, 0, PF_None);
			ClassPtrProp.bIsPointer = true;
			ClassPtrProp.PointerToProp = &ClassProp;
			StandardPtrProperty = &ClassPtrProp;
		}
	}

public:
	static CMetaClass* CreateStatic(const std::string& InName, CController* InController = nullptr)
	{
		if (!InController)
			InController = GetControllerPtr();
		static CMetaClass* MetaClassPtr = [&]() -> CMetaClass* {
			static TClass StaticClass(InName, InController);
			return &StaticClass;
		}();
		return MetaClassPtr;
	}

	static void CopyAssignImpl(void* instance_ptr, const void* other_ptr)
	{
		if constexpr (std::is_copy_assignable_v<T>)
			*static_cast<T*>(instance_ptr) = *static_cast<const T*>(other_ptr);
		else
			THROW_STD_EXCEPTION();
	}

	static void MoveAssignImpl(void* instance_ptr, void* other_ptr)
	{
		if constexpr (std::is_move_assignable_v<T>)
			*static_cast<T*>(instance_ptr) = std::move(*static_cast<T*>(other_ptr));
		else
			THROW_STD_EXCEPTION();
	}

	static void ConstructorImpl(void* instance_ptr)
	{
		if constexpr (std::is_constructible_v<T>)
		{
			if constexpr (!std::is_trivially_constructible_v<T>)
			{
				new (instance_ptr) T();
			}
		}
	}

	static void CopyConstructorImpl(void* instance_ptr, const void* other_ptr)
	{
		if constexpr (std::is_copy_constructible_v<T>)
			new (instance_ptr) T(*static_cast<const T*>(other_ptr));
		else
			THROW_STD_EXCEPTION();
	}

	static void MoveConstructorImpl(void* instance_ptr, void* other_ptr)
	{
		if constexpr (std::is_move_constructible_v<T>)
			new (instance_ptr) T(std::move(*static_cast<T*>(other_ptr)));
		else
			THROW_STD_EXCEPTION();
	}

	static void DestructorImpl(void* instance_ptr)
	{
		if constexpr (!std::is_trivially_destructible_v<T>)
		{
			static_cast<T*>(instance_ptr)->~T();
		}
	}

	virtual void RegisterToScriptEngine(asIScriptEngine* ScriptEngine) 
	{
		if constexpr (!std::is_arithmetic_v<T> && !std::is_void_v<T> && !std::is_same_v<T, std::string>)
		{
			bool IsObject = CastCheckCastRanges(this, OObject::GVar_StaticClass);
			asDWORD ObjectTypeFlag = 0;
			if (IsObject)
			{
				int r = ScriptEngine->RegisterObjectType(Name.c_str(), SizeOf, asOBJ_REF); assert(r >= 0);
				//r = ScriptEngine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asMETHOD(CRef, AddRef), asCALL_THISCALL); assert(r >= 0);
				//r = ScriptEngine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asMETHOD(CRef, Release), asCALL_THISCALL); assert(r >= 0);
			}
			else
			{
				int r = ScriptEngine->RegisterObjectType(Name.c_str(), SizeOf, asOBJ_VALUE | ConvertToScriptEngineTypeTraits()); assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_CONSTRUCT,
					"void f()",
					asFUNCTION(Constructor),
					asCALL_CDECL_OBJLAST); assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_CONSTRUCT,
					fmt::format("void f(const {:s} &in)", Name).c_str(),
					asFUNCTION(CopyConstructor),
					asCALL_CDECL_OBJFIRST); assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_DESTRUCT,
					"void f()",
					asFUNCTION(Destructor),
					asCALL_CDECL_OBJLAST); assert(r >= 0);
				r = ScriptEngine->RegisterObjectMethod(
					Name.c_str(),
					fmt::format("{0:s} &opAssign(const {0:s} &in)", Name).c_str(),
					asMETHODPR(T, operator =, (const T&), T&),
					asCALL_THISCALL); assert(r >= 0);
			}
			auto PropLink = PropertyLink;
			while (PropLink)
			{
				int r = ScriptEngine->RegisterObjectProperty(Name.c_str(), PropLink->GetScriptDeclaration().c_str(), PropLink->Offset); assert(r >= 0);
				PropLink = PropLink->PropertyLinkNext;
			}
			auto FuncLink = FunctionLink;
			while (FuncLink)
			{
				int r = ScriptEngine->RegisterObjectMethod(
					Name.c_str(),
					FuncLink->GetScriptDeclaration().c_str(),
					FuncLink->FuncPtr,
					asCALL_THISCALL); assert(r >= 0);
				FuncLink = FuncLink->FunctionLinkNext;
			}
		}
	}
};

RTCXX_API void SetInheritanceRelationship(CMetaClass* InMetaClass, const char* InBaseClassName, CController* InController);

template<typename T>
FORCEINLINE auto StaticCreateUniqueClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags, const char* InBaseClassName, const char* InInterfaceNames[], CController* InController)
{
	assert(InController);
	static TClass<T> LClass(InOwner, InName, InClassFlags);
	if (InBaseClassName)
	{
		SetInheritanceRelationship(&LClass, InBaseClassName, InController);
	}
	//const char** InterfaceName = InInterfaceNames
	//while (*InterfaceName)
	//{
	//	InterfaceName?
	//	InterfaceName++;
	//}
	InController->RegisterMetadata(&LClass);
	return &LClass;
}

template <typename T, class Enabled = void>
struct TMetaProperty{ using Type = void;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<void       , T>>>{ using Type = CMetaProperty;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<bool       , T>>>{ using Type = CBoolProperty;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<I8         , T>>>{ using Type = CI8Property  ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<I16        , T>>>{ using Type = CI16Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<I32        , T>>>{ using Type = CI32Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<I64        , T>>>{ using Type = CI64Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<U8         , T>>>{ using Type = CU8Property  ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<U16        , T>>>{ using Type = CU16Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<U32        , T>>>{ using Type = CU32Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<U64        , T>>>{ using Type = CU64Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<F32        , T>>>{ using Type = CF32Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<F64        , T>>>{ using Type = CF64Property ;};
template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_same_v<std::string, T>>>{ using Type = CStrProperty ;};

template <typename T>struct TMetaProperty<T, std::enable_if_t<!std::is_same_v<std::string, T> && std::is_class_v<T>>> { using Type = CClassProperty; };

template <typename T>struct TMetaProperty<T, std::enable_if_t<std::is_reference_v<T>>> {
	using PointerToType = std::remove_reference_t<T>;
	using PointerToPropType = typename TMetaProperty<PointerToType>::Type;
	using Type = std::conditional_t<std::is_base_of_v<OObject, PointerToType>, CObjectPtrProperty, CPtrProperty>;
};

template <typename T> struct TMetaProperty<T, std::enable_if_t<std::is_pointer_v<T>>>  {
	using PointerToType = std::remove_pointer_t<T>;
	using PointerToPropType = typename TMetaProperty<PointerToType>::Type;
	using Type = std::conditional_t<std::is_base_of_v<OObject, PointerToType>, CObjectPtrProperty, CPtrProperty>;
};

template <typename T>struct TMetaProperty<T, std::enable_if_t<TIsTArray<T>::Value>>
{
	using ElementType = typename TArrayTraits<T>::ElementType;
	using ElementPropType = typename TMetaProperty<ElementType>::Type;
	using Type = CArrayProperty; 
};

RTCXX_API void SetStandardClassPtrProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController = nullptr);
RTCXX_API void SetStandardClassProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController = nullptr);

template<typename PropertyType>
void SetStandardProperty(CMetaProperty** OutProperty, const char* InClassName, CController* InController)
{
	assert(InController);
	if constexpr (std::is_reference_v<PropertyType> || std::is_pointer_v<PropertyType>)
	{
		SetStandardClassPtrProperty(OutProperty, InClassName, InController);
	}
	else
	{
		SetStandardClassProperty(OutProperty, InClassName, InController);
	}
}

template<> FORCEINLINE void SetStandardProperty<Boolean     >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCBoolProperty;}
template<> FORCEINLINE void SetStandardProperty<I8          >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCI8Property  ;}
template<> FORCEINLINE void SetStandardProperty<I16         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCI16Property ;}
template<> FORCEINLINE void SetStandardProperty<I32         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCI32Property ;}
template<> FORCEINLINE void SetStandardProperty<I64         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCI64Property ;}
template<> FORCEINLINE void SetStandardProperty<U8          >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCU8Property  ;}
template<> FORCEINLINE void SetStandardProperty<U16         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCU16Property ;}
template<> FORCEINLINE void SetStandardProperty<U32         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCU32Property ;}
template<> FORCEINLINE void SetStandardProperty<U64         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCU64Property ;}
template<> FORCEINLINE void SetStandardProperty<F32         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCF32Property ;}
template<> FORCEINLINE void SetStandardProperty<F64         >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCF64Property ;}
template<> FORCEINLINE void SetStandardProperty<std::string >(CMetaProperty** OutProperty, const char* InClassName, CController* InController) { *OutProperty = &StandardCStrProperty ;}

template<I32 Index>
struct TParameterIndex {};

template <typename T>
struct TPropertyGetterTraitsBase
{
	using ClassType = void;
	using PropertyType = std::remove_pointer_t<T>;
};

template <typename C, typename T>
struct TPropertyGetterTraitsBase<T C::*>
{
	using ClassType = C;
	using PropertyType = T;
};

template <typename T>
struct TPropertyGetterTraits : public TPropertyGetterTraitsBase<T>
{
};

// 用于将函数参数类型转换到 TPropertyGetterTraitsBase<T> 可识别的全局指针类型
// 1. 如果是引用, 将其转换为指针
// 2. 再将其转换为此指向此类型的指针
template<typename T>
using TConvertToPropertyGetter = std::add_pointer_t<std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<std::remove_reference_t<T>>, T>>;


// 使用一个本地命名空唯一结构(UnqiueAnonymousType)保证唯一性 
// 命名规则 
// >> >> [1][2]____[3] << <<
// >> [1] 全局为G, 类为C, 函数为F
// >> [2] 属性为P, 类为C, 函数为F
// >> [3] 在所属空间的名称
// 通过模板创建一个唯一的静态属性
template<typename UnqiueAnonymousType, typename PropertyGetterType>
FORCEINLINE auto StaticCreateUniqueProperty(CMetadata* InOwner, const std::string& InName, CMetaProperty::OffsetSizeType InOffset, EPropertyFlags InPropertyFlags, const char* InStandardProperty, CController* InController)
{
	assert(InController);
	using PropertyType = TPropertyGetterTraits<PropertyGetterType>::PropertyType;
	using MetaPropertyType = typename TMetaProperty<PropertyType>::Type;
	static MetaPropertyType LProperty(InOwner, InName, InOffset, InPropertyFlags);
	if constexpr (TIsTArray<PropertyType>::Value)
	{
		using ElementType = typename TMetaProperty<PropertyType>::ElementType;
		using ElementPropType = typename TMetaProperty<PropertyType>::ElementPropType;
		SetStandardProperty<ElementType>(&(LProperty.ElementProp), InStandardProperty, InController);
	}
	if constexpr (std::is_pointer_v<PropertyType> || std::is_reference_v<PropertyType>)
	{
		using PointerToType = typename TMetaProperty<PropertyType>::PointerToType;
		using PointerToPropType = typename TMetaProperty<PropertyType>::PointerToPropType;
		SetStandardProperty<PointerToType>(&(LProperty.PointerToProp), InStandardProperty, InController);
	}
	if (auto OwnerClass = InOwner->CastTo<CMetaClass>())
	{
		OwnerClass->InsertProperty(&LProperty);
	}
	else if (auto OwnerFunction = InOwner->CastTo<CMetaFunction>())
	{
		OwnerFunction->InsertProperty(&LProperty);
	}
	InController->RegisterMetadata(&LProperty);
	return &LProperty;
}
  
// 使用一个本地命名空结构保证唯一性 
// <<< [1][2]____[3] >>> 
// [1] 全局为G, 类为C, 函数为F
// [2] 属性为P, 类为C, 函数为F
// [3] 在所属空间的名称
// 通过模板创建一个唯一的静态属性
template<typename UnqiueAnonymousType, typename OwnerType>
FORCEINLINE auto StaticCreateUniqueFunction(CMetadata* InOwner, const std::string& InName, asSFuncPtr FuncPtr, EFunctionFlags InFunctionFlags, CController* InController)
{
	static CMetaFunction LFunction(InOwner, InName, InFunctionFlags);
	if constexpr (std::is_void_v<OwnerType>)
	{
		LFunction.FuncPtr = FuncPtr;
	}
	else
	{
		LFunction.FuncPtr = FuncPtr;
	}
	if (auto OwnerClass = InOwner->CastTo<CMetaClass>())
	{
		OwnerClass->InsertFunction(&LFunction);
	}
	InController->RegisterMetadata(&LFunction);
	return &LFunction;
}

RTCXX_NAMESPACE_END
