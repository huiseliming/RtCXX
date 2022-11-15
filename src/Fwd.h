#pragma once
#include "RtCXX_export.h"
#include <assert.h>
#include <exception>
#include <fmt/format.h>
#include <source_location>
#include <stdint.h>

#define ASSERT_FORMAT(Expr, ...)

#define THROW_STD_EXCEPTION() throw std::exception(fmt::format("file: {:s}({:d}): `{:s}`", __FILE__, __LINE__, __FUNCTION__).c_str());

#define RTCXX_NAMESPACE_BEGIN \
	namespace RtCXX           \
	{

#define RTCXX_NAMESPACE_END \
	}

#define STRINGIFY(X) #X
#define TO_STRING(x) STRINGIFY(x)

#define SYMBOL_STRING_CAT_IMPL(A, B) A##B
#define SYMBOL_STRING_CAT(A, B)		 SYMBOL_STRING_CAT_IMPL(A, B)

#define FORCEINLINE	  __forceinline		   /* Force code to be inline */
#define FORCENOINLINE __declspec(noinline) /* Force code to NOT be inline */

#define DECL_ENUM_FLAGS(Enum)                                                                                                                                                          \
	inline Enum& operator|=(Enum& LHS, Enum RHS) { return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) | static_cast<std::underlying_type_t<Enum>>(RHS)); }  \
	inline Enum& operator&=(Enum& LHS, Enum RHS) { return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) & static_cast<std::underlying_type_t<Enum>>(RHS)); }  \
	inline Enum& operator^=(Enum& LHS, Enum RHS) { return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) ^ static_cast<std::underlying_type_t<Enum>>(RHS)); }  \
	inline constexpr Enum operator|(Enum LHS, Enum RHS) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) | static_cast<std::underlying_type_t<Enum>>(RHS)); } \
	inline constexpr Enum operator&(Enum LHS, Enum RHS) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) & static_cast<std::underlying_type_t<Enum>>(RHS)); } \
	inline constexpr Enum operator^(Enum LHS, Enum RHS) { return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) ^ static_cast<std::underlying_type_t<Enum>>(RHS)); } \
	inline constexpr bool operator!(Enum E) { return !static_cast<std::underlying_type_t<Enum>>(E); }                                                                                  \
	inline constexpr Enum operator~(Enum E) { return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(E)); }

#if defined(__GNUC__) || defined(__clang__)
	#define DO_PRAGMA(X)					 _Pragma(#X)
	#define PRAGMA_GCC_DIAGNOSTIC_PUSH()	 DO_PRAGMA(GCC diagnostic push)
	#define PRAGMA_GCC_DIAGNOSTIC_WARNING(X) DO_PRAGMA(GCC diagnostic ignored #X)
	#define PRAGMA_GCC_DIAGNOSTIC_POP()		 DO_PRAGMA(GCC diagnostic pop)
	#define PRAGMA_MSVC_WARNING_PUSH()
	#define PRAGMA_MSVC_WARNING_DISABLE(X)
	#define PRAGMA_MSVC_WARNING_POP()
#elif defined(_MSC_VER)
	#define PRAGMA_GCC_DIAGNOSTIC_PUSH()
	#define PRAGMA_GCC_DIAGNOSTIC_WARNING(X)
	#define PRAGMA_GCC_DIAGNOSTIC_POP()
	#define PRAGMA_MSVC_WARNING_PUSH()	   __pragma(warning(push, 0))
	#define PRAGMA_MSVC_WARNING_DISABLE(X) __pragma(warning(disable \
															: X))
	#define PRAGMA_MSVC_WARNING_POP() __pragma(warning(pop))
#else
	#define PRAGMA_GCC_DIAGNOSTIC_PUSH()
	#define PRAGMA_GCC_DIAGNOSTIC_WARNING(X)
	#define PRAGMA_GCC_DIAGNOSTIC_POP()
	#define PRAGMA_MSVC_WARNING_PUSH()
	#define PRAGMA_MSVC_WARNING_DISABLE(X)
	#define PRAGMA_MSVC_WARNING_POP() 
#endif
 
// 
#ifdef ____RTCXX_GENERATOR____
	//#define RMETADATA(NAME) [[clang::annotate("Metadata" __VA_OPT__(", ") #__VA_ARGS__)]] //__attribute__((annotate("Metadata" __VA_OPT__(",") #__VA_ARGS__)))
	#define RCLASS(...)	    [[clang::annotate("Class"    __VA_OPT__(", ") #__VA_ARGS__)]]
	#define RSTRUCT(...)    [[clang::annotate("Struct"   __VA_OPT__(", ") #__VA_ARGS__)]]
	#define RENUM(...)	    [[clang::annotate("Enum"     __VA_OPT__(", ") #__VA_ARGS__)]]
	#define RPROPERTY(...)  [[clang::annotate("Property" __VA_OPT__(", ") #__VA_ARGS__)]]
	#define RFUNCTION(...)  [[clang::annotate("Function" __VA_OPT__(", ") #__VA_ARGS__)]]
#else
	#define RCLASS(...)
	#define RENUM(...)
	#define RPROPERTY(...)
	#define RFUNCTION(...)
	#define RMETADATA(...)
#endif

// 名为`GVar_Static[1]`开头的类静态指针变量是函数`Static[1]`局部静态存储的的全局指针
// 即在应用程序全局初始化之后且在程序main函数返回之前(main函数的作用域)保证可用

#define STATIC_CLASS_DECLARATION                \
public:                                         \
	static RtCXX::CMetaClass* GVar_StaticClass; \
	static RtCXX::CMetaClass* StaticClass();

#define GENERATED_BODY() \
	STATIC_CLASS_DECLARATION

#define GENERATED_CLASS_IMPL(Class, Base)                              \
	RtCXX::CMetaClass* Class::GVar_StaticClass = Class::StaticClass(); \
	RtCXX::CMetaClass* Class::StaticClass()                            \
	{                                                                  \
		using namespace RtCXX;                                         \
		return TClass<Class, Base>::CreateStatic(#Class);              \
	};

#define GENERATED_CLASS_PROPERTY_IMPL(Class, Property) \
	static CMetaProperty* ____##Class##__##Property##____ = TProperty<Class, decltype(&Class::Property)>::CreateStatic(#Property, &Class::Property, offsetof(Class, Property), Class::GVar_StaticClass);

#define GENERATED_CLASS_FUNCTION_IMPL(Class, Function) \
	static CMetaFunction* ____##Class##__##Function##____ = TFunction<decltype(&Class::Function)>::CreateStatic(#Function, &Class::Function, Class::GVar_StaticClass);

struct FNull
{
};

typedef bool _Bool; // libtooling bool type
typedef bool Boolean;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float f32;
typedef double f64;

typedef float F32;
typedef double F64;

template <typename T>
constexpr bool ConstexprTrue = true;
template <typename T>
constexpr bool ConstexprFalse = false;

RTCXX_NAMESPACE_BEGIN

class CController;
class CMetadata;
class CMetaProperty;
class CMetaFunction;
class CMetaClass;

RTCXX_NAMESPACE_END

class OObject;