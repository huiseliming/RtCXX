#pragma once
#include <functional>
#include "Fwd.h"
#include "MetaClass.h"
#include "angelscript.h"
#include "Templates/FunctionTraits.h"
#include "Templates/Null.h"
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

template <typename T>
struct TPropertyTraitsBase
{
	using ClassType = void;
	using PropertyType = std::remove_pointer_t<T>;
	static constexpr bool SIsStatic = true;
	static constexpr bool SIsPointer = std::is_pointer_v<PropertyType>;
	static constexpr bool SIsReference = false; //无法检测到全局变量的引用
};

template <typename C, typename T>
struct TPropertyTraitsBase<T C::*>
{
	using ClassType = C;
	using PropertyType = T;
	static constexpr bool SIsStatic = false;
	static constexpr bool SIsPointer = std::is_pointer_v<PropertyType>;
	static constexpr bool SIsReference = std::is_reference_v<PropertyType>;

};

template <typename T>
struct TPropertyTraits : public TPropertyTraitsBase<T>
{
};

template <typename C /*lass*/, typename E /*xtends*/ = void>
class TClass : public CMetaClass
{
public:
	TClass(const std::string& InName, CController* InController = nullptr)
		: CMetaClass(InName)
	{
		if (!InController)
			InController = GetControllerPtr();
		TypeIndex = typeid(C);
		bHasConstructor = std::is_default_constructible<C>::value && !std::is_trivially_default_constructible<C>::value;
		bHasDestructor = std::is_destructible<C>::value && !std::is_trivially_destructible<C>::value;
		bHasAssignmentOperator = std::is_copy_assignable<C>::value && !std::is_trivially_copy_assignable<C>::value;
		bHasCopyConstructor = std::is_copy_constructible<C>::value && !std::is_trivially_copy_constructible<C>::value;
		
		SizeOf = sizeof(C);

		CopyAssign      = &CopyAssignImpl;
		MoveAssign      = &MoveAssignImpl;
		Constructor     = &ConstructorImpl;
		CopyConstructor = &CopyConstructorImpl;
		MoveConstructor = &MoveConstructorImpl;
		Destructor      = &DestructorImpl;

		if constexpr (std::is_same_v<E, void>)
			ExtendsClass = nullptr;
		else
			InController->GetClassByAfterRegisterClassCallback(typeid(E), [&](CMetaClass* InMetaClass) {
				ExtendsClass = InMetaClass;
				InMetaClass->DerivedClasses.push_back(this);
			});
		if constexpr (std::is_base_of_v<OObject, C>)
		{
			static CObjectProperty ObjProp("Standard" + Name, 0);
			ObjProp.bIsPointer = false;
			ObjProp.MetaClass = this;
			StandardProperty = &ObjProp;
			static CPtrProperty ObjPtrProp("StandardPtr" + Name, 0);
			ObjPtrProp.bIsPointer = true;
			ObjPtrProp.PointerToProp = &ObjProp;
			StandardPtrProperty = &ObjPtrProp;
		}
		else
		{
			static CClassProperty ClassProp("Standard" + Name, 0);
			ClassProp.bIsPointer = false;
			ClassProp.MetaClass = this;
			StandardProperty = &ClassProp;
			static CPtrProperty ClassPtrProp("StandardPtr" + Name, 0);
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
		if constexpr (std::is_copy_assignable_v<C>)
			*static_cast<C*>(instance_ptr) = *static_cast<const C*>(other_ptr);
		else
			THROW_STD_EXCEPTION();
	}

	static void MoveAssignImpl(void* instance_ptr, void* other_ptr)
	{
		if constexpr (std::is_move_assignable_v<C>)
			*static_cast<C*>(instance_ptr) = std::move(*static_cast<C*>(other_ptr));
		else
			THROW_STD_EXCEPTION();
	}

	static void ConstructorImpl(void* instance_ptr)
	{
		if constexpr (std::is_constructible_v<C>)
		{
			if constexpr (!std::is_trivially_constructible_v<C>)
			{
				new (instance_ptr) C();
			}
		}
	}

	static void CopyConstructorImpl(void* instance_ptr, const void* other_ptr)
	{
		if constexpr (std::is_copy_constructible_v<C>)
			new (instance_ptr) C(*static_cast<const C*>(other_ptr));
		else
			THROW_STD_EXCEPTION();
	}

	static void MoveConstructorImpl(void* instance_ptr, void* other_ptr)
	{
		if constexpr (std::is_move_constructible_v<C>)
			new (instance_ptr) C(std::move(*static_cast<C*>(other_ptr)));
		else
			THROW_STD_EXCEPTION();
	}

	static void DestructorImpl(void* instance_ptr)
	{
		if constexpr (!std::is_trivially_destructible_v<C>)
		{
			static_cast<C*>(instance_ptr)->~C();
		}
	}

	virtual void RegisterToScriptEngine(asIScriptEngine* ScriptEngine) 
	{
		if constexpr (!std::is_arithmetic_v<C> && !std::is_void_v<C> && !std::is_same_v<C, std::string>)
		{
			bool IsObject = CastCheckCastRanges(this, OObject::SelfClass);
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
					asMETHODPR(C, operator =, (const C&), C&),
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

template <typename OwnerType, typename P /*ropertyType*/ = void>
struct TProperty
{
	constexpr TProperty(P InProperty)
	{
	
	}
	using ClassType = typename TPropertyTraits<P>::ClassType;
	using PropertyType = typename TPropertyTraits<P>::PropertyType;

	using RemoveRefPropertyType = std::conditional_t<std::is_reference_v<PropertyType>, std::remove_reference_t<PropertyType>, PropertyType>;
	using RemovePtrPropertyType = std::conditional_t<std::is_pointer_v<PropertyType>, std::remove_pointer_t<PropertyType>, PropertyType>;
	static_assert(!std::is_pointer_v<RemoveRefPropertyType>);
	static_assert(!std::is_pointer_v<RemovePtrPropertyType>);
	using PropertyMetaClass = std::conditional_t<std::is_reference_v<PropertyType>, std::remove_reference_t<PropertyType>, std::conditional_t<std::is_pointer_v<PropertyType>, std::remove_pointer_t<PropertyType>, PropertyType>>;

	static constexpr bool SIsStatic = !std::is_member_object_pointer_v<P>;
	static constexpr bool SIsPointer = std::is_pointer_v<PropertyType>;

	constexpr static bool IsStatic() { return SIsStatic; }
	constexpr static bool IsPointer() { return SIsPointer; }

};

template <typename OwnerFunction, typename P /*ropertyType*/ = void>
struct TParameter
{
	using ClassType = typename TPropertyTraits<P>::ClassType;
	using PropertyType = typename TPropertyTraits<P>::PropertyType;

	using RemovePtrPropertyType = std::conditional_t<std::is_pointer_v<PropertyType>, std::remove_pointer_t<PropertyType>, PropertyType>;
	static_assert(!std::is_pointer_v<RemovePtrPropertyType>);

	using PropertyMetaClass = std::conditional_t<std::is_reference_v<PropertyType>, std::remove_reference_t<PropertyType>, std::conditional_t<std::is_pointer_v<PropertyType>, std::remove_pointer_t<PropertyType>, PropertyType>>;

	static constexpr bool SIsStatic = false;
	static constexpr bool SIsPointer = std::is_pointer_v<PropertyType>;

	static CMetaProperty* CreateStatic(const std::string& InName, P InProperty, CMetaProperty::OffsetSizeType InOffsetOf = 0, CMetaFunction* InOwnerFunction = nullptr, CController* InController = nullptr)
	{
		if (!InController)
			InController = GetControllerPtr();
		static CMetaProperty* MetaPropertyPtr = [&]() -> CMetaProperty* {
			static typename TSelectMetaProperty<PropertyMetaClass>::Type StaticMetaProperty(InName);
			StaticMetaProperty.SetOwner(InOwnerFunction);
			StaticMetaProperty.bIsStatic = SIsStatic;
			StaticMetaProperty.bIsPointer = SIsPointer;
			StaticMetaProperty.Offset = InOffsetOf;
			InOwnerFunction->InsertProperty(&StaticMetaProperty);
			if (auto ClassProperty = StaticMetaProperty.CastTo<CClassProperty>())
			{
				InController->GetClassByAfterRegisterClassCallback(typeid(PropertyMetaClass), [ClassProperty](CMetaClass* InPropertyType) {
					ClassProperty->MetaClass = InPropertyType;
					});
			}
			InController->RegisterMetadata(&StaticMetaProperty);
			return &StaticMetaProperty;
		}();
		return MetaPropertyPtr;
	}
};

template <typename F /*unction*/>
struct TFunction
{
	using FunctionType = F;
	using ClassType = typename FunctionTraits<FunctionType>::ClassType;
	using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
	using ArgsType = typename FunctionTraits<FunctionType>::ArgsType;

	template <std::size_t I>
	using TArgType = std::tuple_element_t<I, ArgsType>;

	template <typename T>
	using ConvertRefToPtrType = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<std::remove_reference_t<T>>, T>;

	static constexpr bool SIsStatic = std::is_same_v<void, ClassType>;
	static constexpr bool SIsVoidReturn = std::is_same_v<void, ReturnType>;
	static constexpr i32 SArgCount = FunctionTraits<FunctionType>::SArgCount;

	template <std::size_t I, typename ThisElement>
	static void ForEachParameter(CMetaFunction* InFunction, CController* InController, TNull<ThisElement>&& ThisParameter)
	{
		TParameter<F, ConvertRefToPtrType<ThisElement>*>::CreateStatic(fmt::format("Parameter{:d}", I), static_cast<ConvertRefToPtrType<ThisElement>*>(nullptr), I, InFunction, InController);
	}

	template <std::size_t I, typename ThisElement, typename... Elements>
	static void ForEachParameter(CMetaFunction* InFunction, CController* InController, TNull<ThisElement>&& ThisParameter, TNull<Elements>&&... Parameters)
	{
		ForEachParameter<I, ThisElement>(InFunction, InController, std::forward<TNull<ThisElement>>(ThisParameter));
		ForEachParameter<I + 1, Elements...>(InFunction, InController, std::forward<TNull<Elements>>(Parameters)...);
	}

	template <std::size_t... I>
	static void ForEachParameter(CMetaFunction* InFunction, CController* InController, std::index_sequence<I...>)
	{
		ForEachParameter<1>(InFunction, InController, TNull<std::tuple_element_t<I, ArgsType>>{}...);
	}

	static CMetaFunction* CreateStatic(const std::string& InName, F InFunction, CMetaClass* InOwnerClass = nullptr, CController* InController = nullptr)
	{
		if (!InController)
			InController = GetControllerPtr();
		static CMetaFunction* MetaFunctionPtr = [&]() -> CMetaFunction* {
			static CMetaFunction StaticMetaFunction(InName);
			StaticMetaFunction.bIsStatic = SIsStatic;
			auto RetProp = TParameter<F, ConvertRefToPtrType<ReturnType>*>::CreateStatic("Return", static_cast<ConvertRefToPtrType<ReturnType>*>(nullptr), 0, &StaticMetaFunction, InController);
			RetProp->bIsReturn = true;
			ForEachParameter(&StaticMetaFunction, InController, std::make_index_sequence<SArgCount>());
			if constexpr (SIsStatic)
			{
				StaticMetaFunction.FuncPtr = asFunctionPtr(reinterpret_cast<void (*)()>(static_cast<F>(InFunction)));
			}
			else
			{
				assert(InOwnerClass);
				InOwnerClass->InsertFunction(&StaticMetaFunction);
				StaticMetaFunction.FuncPtr = asSMethodPtr<sizeof(void(ClassType::*)())>::Convert(static_cast<F>(InFunction));
			}
			InController->RegisterMetadata(&StaticMetaFunction);
			return &StaticMetaFunction;
		}();
		return MetaFunctionPtr;
	}
};

RTCXX_NAMESPACE_END
