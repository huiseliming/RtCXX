#pragma once
#include "MetaFunction.h"
#include <typeindex>
RTCXX_NAMESPACE_BEGIN

enum EClassFlags : U64
{
	CF_None					 = 0x0000000000000000,
	CF_HasConstructor		 = 0x1000000000000000,
	CF_HasDestructor		 = 0x2000000000000000,
	CF_HasAssignmentOperator = 0x4000000000000000,
	CF_HasCopyConstructor	 = 0x8000000000000000,
	CF_AllFlags				 = 0xFFFFFFFFFFFFFFFF
};
DECL_ENUM_FLAGS(EClassFlags)

class RTCXX_API CMetaClass : public CMetaStruct
{
	DECLARE_METADATA_CLASS(CMetaClass, CMetaStruct)
public:
	FORCEINLINE CMetaClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags)
		: CMetaStruct(InOwner, InName, CMetaClass::StaticMetadataClass())
		, ClassFlags(InClassFlags)
	{
	}
	FORCEINLINE CMetaClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags, CMetadataClass* InMetadataClass)
		: CMetaStruct(InOwner, InName, InMetadataClass)
		, ClassFlags(InClassFlags)
	{
	}

public:
	EClassFlags ClassFlags;
	FORCEINLINE EClassFlags GetClassFlags() { return ClassFlags; }
	FORCEINLINE void SetClassFlags(EClassFlags NewFlags) { ClassFlags |= NewFlags; }
	FORCEINLINE void ClearClassFlags(EClassFlags NewFlags) { ClassFlags &= ~NewFlags; }
	FORCEINLINE bool HasAnyClassFlags(EClassFlags FlagsToCheck) const { return (ClassFlags & FlagsToCheck) != 0; }
	FORCEINLINE bool HasAllClassFlags(EClassFlags FlagsToCheck) const { return ((ClassFlags & FlagsToCheck) == FlagsToCheck); }

public:
	void InsertFunction(CMetaFunction* InFunction);
	CMetaFunction* FunctionLink = nullptr;

	FORCEINLINE CMetaClass* GetSuperClass() { return static_cast<CMetaClass*>(SuperStruct); }
	FORCEINLINE CMetaClass* GetSubClassLink() { return static_cast<CMetaClass*>(SubStructLink); }
	FORCEINLINE CMetaClass* GetSubClassLinkNext() { return static_cast<CMetaClass*>(SubStructLinkNext); }

public:
	virtual void RegisterToScriptEngine(asIScriptEngine* ScriptEngine) {}

public:
	FORCEINLINE auto New() -> void*
	{
		void* instance_ptr = std::malloc(SizeOf);
		Constructor(instance_ptr);
		return instance_ptr;
	}
	FORCEINLINE auto CopyNew(void* other_ptr) -> void*
	{
		void* instance_ptr = std::malloc(SizeOf);
		CopyConstructor(instance_ptr, other_ptr);
		return instance_ptr;
	}
	FORCEINLINE auto MoveNew(void* other_ptr) -> void*
	{
		void* instance_ptr = std::malloc(SizeOf);
		MoveConstructor(instance_ptr, other_ptr);
		return instance_ptr;
	}
	FORCEINLINE auto Delete(void* instance_ptr) -> void
	{
		Destructor(instance_ptr);
		std::free(instance_ptr);
	}

	static void DefaultCopyAssign(void* instance_ptr, const void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultMoveAssign(void* instance_ptr, void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultConstructor(void* instance_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultCopyConstructor(void* instance_ptr, const void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultMoveConstructor(void* instance_ptr, void* other_ptr) { THROW_STD_EXCEPTION(); }
	static void DefaultDestructor(void* instance_ptr) { THROW_STD_EXCEPTION(); }

	void (*CopyAssign)(void* instance_ptr, const void* other_ptr)	   = &DefaultCopyAssign;
	void (*MoveAssign)(void* instance_ptr, void* other_ptr)			   = &DefaultMoveAssign;
	void (*Constructor)(void* instance_ptr)							   = &DefaultConstructor;
	void (*CopyConstructor)(void* instance_ptr, const void* other_ptr) = &DefaultCopyConstructor;
	void (*MoveConstructor)(void* instance_ptr, void* other_ptr)	   = &DefaultMoveConstructor;
	void (*Destructor)(void* instance_ptr)							   = &DefaultDestructor;

	CMetaProperty* StandardProperty;
	CMetaProperty* StandardPtrProperty;

	// bool IsExtendsFrom(CMetaClass* target_extends_class)
	//{
	//	auto extends_type = BaseClass;
	//	while (extends_type != nullptr)
	//	{
	//		if (extends_type == target_extends_class)
	//			return true;
	//		extends_type = extends_type->BaseClass;
	//	}
	//	return false;
	// }

	std::type_index TypeIndex = typeid(FNull);
	I32 SizeOf;
	std::pair<u32, u32> CastRanges;
};

template <typename T>
class TClass : public CMetaClass
{
public:
	TClass(CMetadata* InOwner, const std::string& InName, EClassFlags InClassFlags)
		: CMetaClass(InOwner, InName, InClassFlags)
	{
		TypeIndex = typeid(T);
		// https://en.cppreference.com/w/cpp/types/is_default_constructible

		if (std::is_default_constructible_v<T> && !std::is_trivially_default_constructible_v<T>)
			SetClassFlags(CF_HasConstructor);
		if (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>)
			SetClassFlags(CF_HasDestructor);
		if (std::is_copy_assignable_v<T> && !std::is_trivially_copy_assignable_v<T>)
			SetClassFlags(CF_HasAssignmentOperator);
		if (std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>)
			SetClassFlags(CF_HasCopyConstructor);

		SizeOf = sizeof(T);

		CopyAssign		= &CopyAssignImpl;
		MoveAssign		= &MoveAssignImpl;
		Constructor		= &ConstructorImpl;
		CopyConstructor = &CopyConstructorImpl;
		MoveConstructor = &MoveConstructorImpl;
		Destructor		= &DestructorImpl;

		static CClassProperty ClassProp(nullptr, "Standard" + Name, 0, PF_None);
		ClassProp.MetaClass = this;
		StandardProperty	= &ClassProp;
		if constexpr (std::is_base_of_v<OObject, T>)
		{
			static CObjectPtrProperty ObjPtrProp(nullptr, "StandardPtr" + Name, 0, PF_None);
			ObjPtrProp.PointerToProp = &ClassProp;
			StandardPtrProperty		 = &ObjPtrProp;
		}
		else
		{
			static CPtrProperty ClassPtrProp(nullptr, "StandardPtr" + Name, 0, PF_None);
			ClassPtrProp.PointerToProp = &ClassProp;
			StandardPtrProperty		   = &ClassPtrProp;
		}
	}

public:
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
			bool IsObject		   = CastCheckCastRanges(this, OObject::GVar_StaticClass);
			asDWORD ObjectTypeFlag = 0;
			if (IsObject)
			{
				int r = ScriptEngine->RegisterObjectType(Name.c_str(), SizeOf, asOBJ_REF);
				assert(r >= 0);
				// r = ScriptEngine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asMETHOD(CRef, AddRef), asCALL_THISCALL); assert(r >= 0);
				// r = ScriptEngine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asMETHOD(CRef, Release), asCALL_THISCALL); assert(r >= 0);
			}
			else
			{
				asDWORD TypeTraits = asOBJ_APP_CLASS;
				if (HasAnyClassFlags(CF_HasConstructor))
					TypeTraits |= asOBJ_APP_CLASS_CONSTRUCTOR;
				if (HasAnyClassFlags(CF_HasDestructor))
					TypeTraits |= asOBJ_APP_CLASS_DESTRUCTOR;
				if (HasAnyClassFlags(CF_HasAssignmentOperator))
					TypeTraits |= asOBJ_APP_CLASS_ASSIGNMENT;
				if (HasAnyClassFlags(CF_HasCopyConstructor))
					TypeTraits |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;
				int r = ScriptEngine->RegisterObjectType(Name.c_str(), SizeOf, asOBJ_VALUE | TypeTraits);
				assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_CONSTRUCT,
					"void f()",
					asFUNCTION(Constructor),
					asCALL_CDECL_OBJLAST);
				assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_CONSTRUCT,
					fmt::format("void f(const {:s} &in)", Name).c_str(),
					asFUNCTION(CopyConstructor),
					asCALL_CDECL_OBJFIRST);
				assert(r >= 0);
				r = ScriptEngine->RegisterObjectBehaviour(
					Name.c_str(),
					asBEHAVE_DESTRUCT,
					"void f()",
					asFUNCTION(Destructor),
					asCALL_CDECL_OBJLAST);
				assert(r >= 0);
				r = ScriptEngine->RegisterObjectMethod(
					Name.c_str(),
					fmt::format("{0:s} &opAssign(const {0:s} &in)", Name).c_str(),
					asMETHODPR(T, operator=, (const T&), T&),
					asCALL_THISCALL);
				assert(r >= 0);
			}
			auto PropLink = PropertyLink;
			while (PropLink)
			{
				int r = ScriptEngine->RegisterObjectProperty(Name.c_str(), PropLink->GetDeclarationForScriptEngine().c_str(), PropLink->Offset);
				assert(r >= 0);
				PropLink = PropLink->PropertyLinkNext;
			}
			auto FuncLink = FunctionLink;
			while (FuncLink)
			{
				int r = ScriptEngine->RegisterObjectMethod(
					Name.c_str(),
					FuncLink->GetDeclarationForScriptEngine().c_str(),
					FuncLink->FuncPtr,
					asCALL_THISCALL);
				assert(r >= 0);
				FuncLink = FuncLink->FunctionLinkNext;
			}
		}
	}
};

RTCXX_NAMESPACE_END