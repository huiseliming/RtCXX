#include "MetaFunction.h"
#include "MetaClass.h"

RTCXX_NAMESPACE_BEGIN

IMPLEMENT_METADATA_CLASS(CMetaFunction)

std::string CMetaFunction::GetDeclarationForScriptEngine()
{
	std::string Declaration;
	auto PropertyLinkIt = PropertyLink;
	std::string DeclarationReturn;
	std::vector<std::string> DeclarationArgs;

	while (PropertyLinkIt)
	{
		if (PropertyLinkIt->HasAnyPropertyFlags(PF_ReturnParam))
		{
			DeclarationReturn = PropertyLinkIt->GetTypeDeclaration();
		}
		else
		{
			DeclarationArgs.push_back(PropertyLinkIt->GetTypeDeclaration());
			if (DeclarationArgs.back().back() == '&')
			{

				DeclarationArgs.back().append(PropertyLinkIt->HasAnyPropertyFlags(EPropertyFlags::PF_OutParam) ? " out" : " in");
			}
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

RTCXX_NAMESPACE_END