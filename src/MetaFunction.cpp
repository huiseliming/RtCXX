#include "MetaFunction.h"
#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

std::string CMetaFunction::GetScriptDeclaration()
{
	std::string Declaration;
	auto PropertyLinkIt = PropertyLink;
	std::string DeclarationReturn;
	std::vector<std::string> DeclarationArgs;

	while (PropertyLinkIt)
	{
		if (PropertyLinkIt->bIsReturn)
		{
			DeclarationReturn = PropertyLinkIt->GetScriptTypeDecl() + (PropertyLinkIt->bIsPointer ? "&" : "");
		}
		else
		{
			DeclarationArgs.push_back(PropertyLinkIt->GetScriptTypeDecl() + (PropertyLinkIt->bIsPointer ? (PropertyLinkIt->bIsOut ? "&out" : "&in") : ""));
		}
		PropertyLinkIt = PropertyLinkIt->PropertyLinkNext;
	}

	Declaration = DeclarationReturn + " " + GetName() + "(";
	if (DeclarationArgs.size() > 0)
	{
		Declaration += DeclarationArgs[0];
	}
	for (size_t i = 1; i < DeclarationArgs.size(); i++)
	{
		Declaration += ", ";
		Declaration += DeclarationArgs[i];
	}
	Declaration += ")";
    return Declaration;
}

void CMetaFunction::InsertProperty(CMetaProperty* InMetaProperty)
{
	if (!PropertyLink)
	{
		PropertyLink = InMetaProperty;
		return;
	}
	InMetaProperty->PropertyLinkNext = PropertyLink;
	PropertyLink = InMetaProperty;
}

RTCXX_NAMESPACE_END