#include "MetaProperty.h"
#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

CBoolProperty StandardCBoolProperty("StandardCBoolProperty", 0);
CNumericProperty StandardCNumericProperty("StandardCNumericProperty", 0);
CI8Property StandardCI8Property("StandardCI8Property", 0);
CI16Property StandardCI16Property("StandardCI16Property", 0);
CI32Property StandardCI32Property("StandardCI32Property", 0);
CI64Property StandardCI64Property("StandardCI64Property", 0);
CU8Property StandardCU8Property("StandardCU8Property", 0);
CU16Property StandardCU16Property("StandardCU16Property", 0);
CU32Property StandardCU32Property("StandardCU32Property", 0);
CU64Property StandardCU64Property("StandardCU64Property", 0);
CF32Property StandardCF32Property("StandardCF32Property", 0);
CF64Property StandardCF64Property("StandardCF64Property", 0);
CStrProperty StandardCStrProperty("StandardCStrProperty", 0);
// CClassProperty StandardCClassProperty("StandardCClassProperty", 0);
// CObjectProperty StandardCObjectProperty("StandardCObjectProperty", 0);
// CObjectPtrProperty StandardCObjectPtrProperty("StandardCObjectPtrProperty", 0);
// CArrayProperty StandardCArrayProperty("StandardCArrayProperty", 0);

CPtrProperty StandardPtrCBoolProperty("StandardPtrCBoolProperty", 0);
CPtrProperty StandardPtrCNumericProperty("StandardPtrCNumericProperty", 0);
CPtrProperty StandardPtrCI8Property("StandardPtrCI8Property", 0);
CPtrProperty StandardPtrCI16Property("StandardPtrCI16Property", 0);
CPtrProperty StandardPtrCI32Property("StandardPtrCI32Property", 0);
CPtrProperty StandardPtrCI64Property("StandardPtrCI64Property", 0);
CPtrProperty StandardPtrCU8Property("StandardPtrCU8Property", 0);
CPtrProperty StandardPtrCU16Property("StandardPtrCU16Property", 0);
CPtrProperty StandardPtrCU32Property("StandardPtrCU32Property", 0);
CPtrProperty StandardPtrCU64Property("StandardPtrCU64Property", 0);
CPtrProperty StandardPtrCF32Property("StandardPtrCF32Property", 0);
CPtrProperty StandardPtrCF64Property("StandardPtrCF64Property", 0);
CPtrProperty StandardPtrCStrProperty("StandardPtrCStrProperty", 0);
// CClassProperty StandardPtrCClassProperty("StandardPtrCClassProperty", 0);
// CObjectProperty StandardPtrCObjectProperty("StandardPtrCObjectProperty", 0);
// CObjectPtrProperty StandardPtrCObjectPtrProperty("StandardPtrCObjectPtrProperty", 0);
// CArrayProperty StandardPtrCArrayProperty("StandardPtrCArrayProperty", 0);

static struct CStandardPropertyIniter
{
	CStandardPropertyIniter()
	{
		StandardCBoolProperty.bIsPointer = false;
		StandardCNumericProperty.bIsPointer = false;
		StandardCI8Property.bIsPointer = false;
		StandardCI16Property.bIsPointer = false;
		StandardCI32Property.bIsPointer = false;
		StandardCI64Property.bIsPointer = false;
		StandardCU8Property.bIsPointer = false;
		StandardCU16Property.bIsPointer = false;
		StandardCU32Property.bIsPointer = false;
		StandardCU64Property.bIsPointer = false;
		StandardCF32Property.bIsPointer = false;
		StandardCF64Property.bIsPointer = false;
		StandardCStrProperty.bIsPointer = false;
		// StandardCClassProperty.bIsPointer = false;
		// StandardCObjectProperty.bIsPointer = false;
		// StandardCObjectPtrProperty.bIsPointer = false;
		// StandardCArrayProperty.bIsPointer = false;

		StandardPtrCBoolProperty.PointerToProp = &StandardCBoolProperty;
		StandardPtrCNumericProperty.PointerToProp = &StandardCNumericProperty;
		StandardPtrCI8Property.PointerToProp = &StandardCI8Property;
		StandardPtrCI16Property.PointerToProp = &StandardCI16Property;
		StandardPtrCI32Property.PointerToProp = &StandardCI32Property;
		StandardPtrCI64Property.PointerToProp = &StandardCI64Property;
		StandardPtrCU8Property.PointerToProp = &StandardCU8Property;
		StandardPtrCU16Property.PointerToProp = &StandardCU16Property;
		StandardPtrCU32Property.PointerToProp = &StandardCU32Property;
		StandardPtrCU64Property.PointerToProp = &StandardCU64Property;
		StandardPtrCF32Property.PointerToProp = &StandardCF32Property;
		StandardPtrCF64Property.PointerToProp = &StandardCF64Property;
		StandardPtrCStrProperty.PointerToProp = &StandardCStrProperty;

		StandardPtrCBoolProperty.bIsPointer = true;
		StandardPtrCNumericProperty.bIsPointer = true;
		StandardPtrCI8Property.bIsPointer = true;
		StandardPtrCI16Property.bIsPointer = true;
		StandardPtrCI32Property.bIsPointer = true;
		StandardPtrCI64Property.bIsPointer = true;
		StandardPtrCU8Property.bIsPointer = true;
		StandardPtrCU16Property.bIsPointer = true;
		StandardPtrCU32Property.bIsPointer = true;
		StandardPtrCU64Property.bIsPointer = true;
		StandardPtrCF32Property.bIsPointer = true;
		StandardPtrCF64Property.bIsPointer = true;
		StandardPtrCStrProperty.bIsPointer = true;

		// StandardPtrCClassProperty.bIsPointer = true;
		// StandardPtrCObjectProperty.bIsPointer = true;
		// StandardPtrCObjectPtrProperty.bIsPointer = true;
		// StandardPtrCArrayProperty.bIsPointer = true;
	}
} StandardPropertyIniter;

std::string CMetaProperty::GetScriptDeclaration()
{
	return fmt::format("{:s} {:s}{:s}", GetScriptTypeDecl(), bIsPointer ? "&" : "", Name);
}

std::string CBoolProperty::GetScriptTypeDecl()
{
	return "bool";
}

std::string CI8Property::GetScriptTypeDecl()
{
	return "int8";
}

std::string CI16Property::GetScriptTypeDecl()
{
	return "int16";
}

std::string CI32Property::GetScriptTypeDecl()
{
	return "int";
}

std::string CI64Property::GetScriptTypeDecl()
{
	return "int64";
}

std::string CU8Property::GetScriptTypeDecl()
{
	return "uint8";
}

std::string CU16Property::GetScriptTypeDecl()
{
	return "uint16";
}

std::string CU32Property::GetScriptTypeDecl()
{
	return "uint";
}

std::string CU64Property::GetScriptTypeDecl()
{
	return "uint64";
}

std::string CF32Property::GetScriptTypeDecl()
{
	return "float";
}

std::string CF64Property::GetScriptTypeDecl()
{
	return "double";
}

std::string CClassProperty::GetScriptTypeDecl()
{
	return MetaClass->GetName();
}

std::string CStrProperty::GetScriptTypeDecl()
{
	return "string";
}

RTCXX_NAMESPACE_END
