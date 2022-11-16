#include "MetaProperty.h"
#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

IMPLEMENT_METADATA_CLASS(CMetaProperty)
IMPLEMENT_METADATA_CLASS(CBoolProperty)
IMPLEMENT_METADATA_CLASS(CNumericProperty)
IMPLEMENT_METADATA_CLASS(CI8Property)
IMPLEMENT_METADATA_CLASS(CI16Property)
IMPLEMENT_METADATA_CLASS(CI32Property)
IMPLEMENT_METADATA_CLASS(CI64Property)
IMPLEMENT_METADATA_CLASS(CU8Property)
IMPLEMENT_METADATA_CLASS(CU16Property)
IMPLEMENT_METADATA_CLASS(CU32Property)
IMPLEMENT_METADATA_CLASS(CU64Property)
IMPLEMENT_METADATA_CLASS(CF32Property)
IMPLEMENT_METADATA_CLASS(CF64Property)
IMPLEMENT_METADATA_CLASS(CStrProperty)
IMPLEMENT_METADATA_CLASS(CClassProperty)
IMPLEMENT_METADATA_CLASS(CPtrProperty)
IMPLEMENT_METADATA_CLASS(CObjectPtrProperty)
IMPLEMENT_METADATA_CLASS(CArrayProperty)

CBoolProperty StandardCBoolProperty(nullptr, "StandardCBoolProperty", 0, PF_None);
CNumericProperty StandardCNumericProperty(nullptr, "StandardCNumericProperty", 0, PF_None);
CI8Property StandardCI8Property(nullptr, "StandardCI8Property", 0, PF_None);
CI16Property StandardCI16Property(nullptr, "StandardCI16Property", 0, PF_None);
CI32Property StandardCI32Property(nullptr, "StandardCI32Property", 0, PF_None);
CI64Property StandardCI64Property(nullptr, "StandardCI64Property", 0, PF_None);
CU8Property StandardCU8Property(nullptr, "StandardCU8Property", 0, PF_None);
CU16Property StandardCU16Property(nullptr, "StandardCU16Property", 0, PF_None);
CU32Property StandardCU32Property(nullptr, "StandardCU32Property", 0, PF_None);
CU64Property StandardCU64Property(nullptr, "StandardCU64Property", 0, PF_None);
CF32Property StandardCF32Property(nullptr, "StandardCF32Property", 0, PF_None);
CF64Property StandardCF64Property(nullptr, "StandardCF64Property", 0, PF_None);
CStrProperty StandardCStrProperty(nullptr, "StandardCStrProperty", 0, PF_None);
// CClassProperty StandardCClassProperty(nullptr, "StandardCClassProperty", 0, PF_None);
// CObjectProperty StandardCObjectProperty(nullptr, "StandardCObjectProperty", 0, PF_None);
// CObjectPtrProperty StandardCObjectPtrProperty(nullptr, "StandardCObjectPtrProperty", 0, PF_None);
// CArrayProperty StandardCArrayProperty(nullptr, "StandardCArrayProperty", 0, PF_None);

CPtrProperty StandardPtrCBoolProperty(nullptr, "StandardPtrCBoolProperty", 0, PF_None);
CPtrProperty StandardPtrCNumericProperty(nullptr, "StandardPtrCNumericProperty", 0, PF_None);
CPtrProperty StandardPtrCI8Property(nullptr, "StandardPtrCI8Property", 0, PF_None);
CPtrProperty StandardPtrCI16Property(nullptr, "StandardPtrCI16Property", 0, PF_None);
CPtrProperty StandardPtrCI32Property(nullptr, "StandardPtrCI32Property", 0, PF_None);
CPtrProperty StandardPtrCI64Property(nullptr, "StandardPtrCI64Property", 0, PF_None);
CPtrProperty StandardPtrCU8Property(nullptr, "StandardPtrCU8Property", 0, PF_None);
CPtrProperty StandardPtrCU16Property(nullptr, "StandardPtrCU16Property", 0, PF_None);
CPtrProperty StandardPtrCU32Property(nullptr, "StandardPtrCU32Property", 0, PF_None);
CPtrProperty StandardPtrCU64Property(nullptr, "StandardPtrCU64Property", 0, PF_None);
CPtrProperty StandardPtrCF32Property(nullptr, "StandardPtrCF32Property", 0, PF_None);
CPtrProperty StandardPtrCF64Property(nullptr, "StandardPtrCF64Property", 0, PF_None);
CPtrProperty StandardPtrCStrProperty(nullptr, "StandardPtrCStrProperty", 0, PF_None);
// CClassProperty StandardPtrCClassProperty(nullptr, "StandardPtrCClassProperty", 0, PF_None);
// CObjectProperty StandardPtrCObjectProperty(nullptr, "StandardPtrCObjectProperty", 0, PF_None);
// CObjectPtrProperty StandardPtrCObjectPtrProperty(nullptr, "StandardPtrCObjectPtrProperty", 0, PF_None);
// CArrayProperty StandardPtrCArrayProperty(nullptr, "StandardPtrCArrayProperty", 0, PF_None);

static struct CStandardPropertyIniter
{
	CStandardPropertyIniter()
	{

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
	}
} StandardPropertyIniter;

std::string CMetaProperty::GetDeclarationForScriptEngine()
{
	return fmt::format("{:s} {:s}", GetTypeDeclaration(), Name);
}

std::string CBoolProperty::GetTypeDeclaration()
{
	return "bool";
}

std::string CI8Property::GetTypeDeclaration()
{
	return "int8";
}

std::string CI16Property::GetTypeDeclaration()
{
	return "int16";
}

std::string CI32Property::GetTypeDeclaration()
{
	return "int";
}

std::string CI64Property::GetTypeDeclaration()
{
	return "int64";
}

std::string CU8Property::GetTypeDeclaration()
{
	return "uint8";
}

std::string CU16Property::GetTypeDeclaration()
{
	return "uint16";
}

std::string CU32Property::GetTypeDeclaration()
{
	return "uint";
}

std::string CU64Property::GetTypeDeclaration()
{
	return "uint64";
}

std::string CF32Property::GetTypeDeclaration()
{
	return "float";
}

std::string CF64Property::GetTypeDeclaration()
{
	return "double";
}

std::string CStrProperty::GetTypeDeclaration()
{
	return "string";
}

std::string CClassProperty::GetTypeDeclaration()
{
	return MetaClass->GetName();
}

std::string CPtrProperty::GetTypeDeclaration() {
	return PointerToProp->GetTypeDeclaration() + "&";
}

RTCXX_NAMESPACE_END

