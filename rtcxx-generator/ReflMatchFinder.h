#pragma once
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <filesystem>
#include <fstream>

extern std::unordered_set<std::string> HeaderFiles;
extern std::string RtCXXCppFileContext;
#define DECLARE_METADATA_STATIC_CAST_FLAG(Class) \
	constexpr static ECastId StaticECastId() { return ECastId(CI_##Class); }

#define DECL_CAST(Class, SuperClass)                                   \
public:                                                                       \
	DECLARE_METADATA_STATIC_CAST_FLAG(Class)                                  \
	Class()                                                                   \
		: SuperClass()                                                        \
	{                                                                         \
		CastId = ECastId(CastId | Class::StaticECastId()); \
	}                                                                         \

enum ECastId {

    CI_None                         = 0x00000000,
    CI_CPropertyConstructor         = 0x00000001,
    CI_CBoolPropertyConstructor     = 0x00000002,
    CI_CI8PropertyConstructor       = 0x00000004,
    CI_CI16PropertyConstructor      = 0x00000008,
    CI_CI32PropertyConstructor      = 0x00000010,
    CI_CI64PropertyConstructor      = 0x00000020,
    CI_CU8PropertyConstructor       = 0x00000040,
    CI_CU16PropertyConstructor      = 0x00000080,
    CI_CU32PropertyConstructor      = 0x00000100,
    CI_CU64PropertyConstructor      = 0x00000200,
    CI_CF32PropertyConstructor      = 0x00000400,
    CI_CF64PropertyConstructor      = 0x00000800,
    CI_CEnumPropertyConstructor     = 0x00001000,
    CI_CClassPropertyConstructor    = 0x00002000,
    CI_CStrPropertyConstructor      = 0x00004000,
    CI_CPtrPropertyConstructor      = 0x00008000,
    CI_CObjectPtrPropertyConstructor= 0x00010000,
    CI_CArrayPropertyConstructor    = 0x00020000,
};

struct CPropertyConstructor {
    constexpr static ECastId StaticECastId() {
        return ECastId(CI_CPropertyConstructor);
    }
    ECastId CastId;
    CPropertyConstructor()
        : CastId(CI_None)
    {
        CastId = ECastId(CastId | StaticECastId());
    }
    CPropertyConstructor(ECastId InCastId)
        :CastId(CI_CPropertyConstructor)
    {}

    template<typename T>
    T* CastTo()
    {
        if (CastId & T::StaticECastId())
            return static_cast<T*>(this);
        return nullptr;
    }

  virtual ~CPropertyConstructor() {}
  virtual std::string GetPropertyType() { return "CMetaProperty"; }

  const clang::NamedDecl* OwnerDecl;
  const clang::NamedDecl* PropertyDecl;
};

struct CBoolPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CBoolPropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CBoolProperty"; }
};

struct CI8PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CI8PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CI8Property"; }
};
struct CI16PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CI16PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CI16Property"; }
};
struct CI32PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CI32PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CI32Property"; }
};
struct CI64PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CI64PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CI64Property"; }
};
struct CU8PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CU8PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CU8Property"; }
};
struct CU16PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CU16PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CU16Property"; }
};
struct CU32PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CU32PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CU32Property"; }
};
struct CU64PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CU64PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CU64Property"; }
};
struct CF32PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CF32PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CF32Property"; }
};
struct CF64PropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CF64PropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CF64Property"; }
};
struct CClassPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CClassPropertyConstructor, CPropertyConstructor)
    virtual std::string GetPropertyType() { return "CClassProperty"; }
    clang::CXXRecordDecl* ClassDecl;
};
struct CEnumPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CEnumPropertyConstructor, CPropertyConstructor)
    virtual std::string GetPropertyType() { return "CEnumProperty"; }
    std::string EnumName;
};
struct CStrPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CStrPropertyConstructor, CPropertyConstructor)
  virtual std::string GetPropertyType() { return "CStrProperty"; }
};

struct CPtrPropertyConstructor : public CPropertyConstructor
{
    DECL_CAST(CPtrPropertyConstructor, CPropertyConstructor)
    std::shared_ptr<CPropertyConstructor> PointerToProp;
    virtual std::string GetPropertyType() { return "CPtrProperty"; }
};

struct CObjectPtrPropertyConstructor : public CPtrPropertyConstructor {
    DECL_CAST(CObjectPtrPropertyConstructor, CPtrPropertyConstructor)
    virtual std::string GetPropertyType() { return "CObjectPtrProperty"; }
};

struct CArrayPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CArrayPropertyConstructor, CPropertyConstructor)
    virtual std::string GetPropertyType() { return "CArrayProperty"; }
    std::shared_ptr<CPropertyConstructor> ElementProp;
};

class ReflClassMatchFinder
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult& MatchResult) {
        Context = MatchResult.Context;
        RtCXXSourceManager = MatchResult.SourceManager;

        clang::Decl const* Decl = MatchResult.Nodes.getNodeAs<clang::Decl>("Decl");
        auto Filename = RtCXXSourceManager->getFilename(Decl->getLocation());
        llvm::SmallVector<char, 1024> SmallVector(Filename.begin(), Filename.end());
        std::unordered_set<unsigned int> FileIDSet;
        for (auto& HeaderFile : HeaderFiles)
        {
            auto ExpectedFileRef = RtCXXSourceManager->getFileManager().getFileRef(HeaderFile);
            if (ExpectedFileRef)
            {
                auto FileRef = ExpectedFileRef.get();
                FileIDSet.insert(FileRef.getFileEntry().getUID());
            }
        }
        //RtCXXSourceManager->fileid
        if (FileIDSet.contains(RtCXXSourceManager->getFileEntryRefForID(RtCXXSourceManager->getFileID(Decl->getLocation())).getValue().getUID())) {
            if (clang::EnumDecl const* EnumDecl =
                MatchResult.Nodes.getNodeAs<clang::EnumDecl>("Decl")) {
                EnumDecls.push_back(EnumDecl);
            }

            if (clang::CXXRecordDecl const* CXXRecordDecl =
                MatchResult.Nodes.getNodeAs<clang::CXXRecordDecl>("Decl")) {
                CXXRecordDecls.insert(CXXRecordDecl);
            }

            if (clang::FieldDecl const* FieldDecl =
                MatchResult.Nodes.getNodeAs<clang::FieldDecl>("Decl")) {
                clang::CXXRecordDecl const* Parent =
                    cast<clang::CXXRecordDecl const>(FieldDecl->getParent());
                FieldDeclMap.insert(std::make_pair(FieldDecl, Parent));
            }

            if (clang::FunctionDecl const* FunctionDecl =
                MatchResult.Nodes.getNodeAs<clang::FunctionDecl>("Decl")) {
                clang::CXXRecordDecl const* Parent =
                    cast<clang::CXXRecordDecl const>(FunctionDecl->getParent());
                FunctionDeclMap.insert(std::make_pair(FunctionDecl, Parent));
            }
        }
    }

    bool ParseReflectAnnotation(
        const clang::Decl* CheckedDecl,
        std::unordered_map<std::string, std::string>& OutMetadata) {
        OutMetadata.clear();
        if (CheckedDecl->hasAttrs()) {
            for (auto Attr : CheckedDecl->getAttrs()) {

                if (Attr->getKind() == clang::attr::Annotate) {
                    std::string RowStringBuffer;
                    llvm::raw_string_ostream RowStringOutputStream(RowStringBuffer);
                    Attr->printPretty(RowStringOutputStream,
                        clang::PrintingPolicy(clang::LangOptions()));
                    std::string AttrFullString(RowStringOutputStream.str());
                    constexpr static size_t ReflAnnotateStartSize =
                        sizeof("[[clang::annotate(\"RtCXX") - 1;
                    constexpr static size_t ReflAnnotateEndSize = sizeof("\")]]") - 1;
                    size_t ReflAnnotateStartPos =
                        AttrFullString.find("[[clang::annotate(\"RtCXX");
                    size_t ReflAnnotateEndPos = AttrFullString.rfind("\")]]");
                    if (ReflAnnotateStartPos != std::string::npos &&
                        ReflAnnotateEndPos != std::string::npos &&
                        ReflAnnotateStartPos <= 1 &&
                        (ReflAnnotateEndPos == (AttrFullString.size() - 4))) {
                        std::string MetaString;
                        MetaString.reserve(AttrFullString.size() - ReflAnnotateStartSize -
                            ReflAnnotateEndSize - ReflAnnotateStartPos);
                        std::string MetadataKey, MetadataValue;
                        size_t OffsetIndex = ReflAnnotateStartSize + ReflAnnotateStartPos;
                        size_t LastIndex = AttrFullString.size() - ReflAnnotateEndSize;
                        if (AttrFullString[OffsetIndex] == ',')
                            OffsetIndex++;
                        while (OffsetIndex < LastIndex) {
                            while (OffsetIndex < LastIndex &&
                                std::isspace(AttrFullString[OffsetIndex]))
                                OffsetIndex++;
                            while (OffsetIndex < LastIndex &&
                                (std::isalpha(AttrFullString[OffsetIndex]) ||
                                    std::isdigit(AttrFullString[OffsetIndex]) ||
                                    AttrFullString[OffsetIndex] == '_')) {
                                MetadataKey.push_back(AttrFullString[OffsetIndex]);
                                OffsetIndex++;
                            }
                            while (OffsetIndex < LastIndex &&
                                std::isspace(AttrFullString[OffsetIndex]))
                                OffsetIndex++;
                            if (AttrFullString[OffsetIndex] == '=') {
                                OffsetIndex++;
                                while (OffsetIndex < LastIndex &&
                                    std::isspace(AttrFullString[OffsetIndex]))
                                    OffsetIndex++;
                                if (AttrFullString[OffsetIndex] == '\"') {
                                    OffsetIndex++;
                                    while (OffsetIndex < LastIndex &&
                                        AttrFullString[OffsetIndex] != '\"') {
                                        if (AttrFullString[OffsetIndex] == '\\') {
                                            if (AttrFullString[OffsetIndex] == '\\' ||
                                                AttrFullString[OffsetIndex] == '\"') {
                                                MetadataValue.push_back(AttrFullString[OffsetIndex]);
                                                OffsetIndex++;
                                            }
                                            else {
                                                throw std::runtime_error(std::format(
                                                    "{:s} illegal metadata macro, correct format "
                                                    "should be RMETADATA(A=B, C = \"D\")",
                                                    Attr->getLocation().printToString(
                                                        *RtCXXSourceManager)));
                                            }
                                        }
                                        MetadataValue.push_back(AttrFullString[OffsetIndex]);
                                        OffsetIndex++;
                                    }
                                    OffsetIndex++;
                                }
                                else {
                                    if (std::isdigit(AttrFullString[OffsetIndex])) {
                                        while (OffsetIndex < LastIndex &&
                                            (std::isdigit(AttrFullString[OffsetIndex]) ||
                                                AttrFullString[OffsetIndex] != '.')) {
                                            MetadataValue.push_back(AttrFullString[OffsetIndex]);
                                            OffsetIndex++;
                                        }
                                    }
                                    else {
                                        while (OffsetIndex < LastIndex &&
                                            (std::isalpha(AttrFullString[OffsetIndex]) ||
                                                std::isdigit(AttrFullString[OffsetIndex]) ||
                                                AttrFullString[OffsetIndex] == '_')) {
                                            MetadataValue.push_back(AttrFullString[OffsetIndex]);
                                            OffsetIndex++;
                                        }
                                    }
                                }
                                while (OffsetIndex < LastIndex &&
                                    std::isspace(AttrFullString[OffsetIndex]))
                                    OffsetIndex++;
                                if (OutMetadata.contains(MetadataKey)) {
                                    throw std::runtime_error(std::format(
                                        "{:s} illegal metadata macro, correct format should be "
                                        "RMETADATA(A=B, C = \"D\")",
                                        Attr->getLocation().printToString(*RtCXXSourceManager)));
                                }
                                OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
                                if (AttrFullString[OffsetIndex] == ',') {
                                    OffsetIndex++;
                                }
                                else if (OffsetIndex == LastIndex) {

                                }
                                else {
                                    throw std::runtime_error(std::format(
                                        "{:s} illegal metadata macro, correct format should be "
                                        "RMETADATA(A=B, C = \"D\")",
                                        Attr->getLocation().printToString(*RtCXXSourceManager)));
                                }
                            }
                            else if (AttrFullString[OffsetIndex] == ',') {
                                if (OutMetadata.contains(MetadataKey)) {
                                    throw std::runtime_error(std::format(
                                        "{:s} illegal metadata macro, correct format should be "
                                        "RMETADATA(A=B, C = \"D\")",
                                        Attr->getLocation().printToString(*RtCXXSourceManager)));
                                }
                                OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
                                OffsetIndex++;
                            }
                            else if (OffsetIndex == LastIndex) {
                                if (OutMetadata.contains(MetadataKey)) {
                                    throw std::runtime_error(std::format(
                                        "{:s} illegal metadata macro, correct format should be "
                                        "RMETADATA(A=B, C = \"D\")",
                                        Attr->getLocation().printToString(*RtCXXSourceManager)));
                                }
                                OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
                            }
                            else {
                                throw std::runtime_error(std::format(
                                    "{:s} illegal metadata macro, correct format should be "
                                    "RMETADATA(A=B, C = \"D\")",
                                    Attr->getLocation().printToString(*RtCXXSourceManager)));
                            }
                            MetadataKey.clear();
                            MetadataValue.clear();
                        }
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void
        GetMetadataString(const std::string& OutDeclName,
            const std::map<std::string, std::string>& InMetadataMap,
            std::string& OutMetadataDefineString) {
        OutMetadataDefineString +=
            std::format("static std::array<std::pair<std::string, std::string>, "
                "{:d}> {:s} = {{\n",
                InMetadataMap.size(), OutDeclName);
        for (auto Metadata : InMetadataMap) {
            OutMetadataDefineString +=
                std::format("    std::pair{{\"{:s}\", \"{:s}\"}},\n", Metadata.first,
                    Metadata.second);
        }
        OutMetadataDefineString.resize(OutMetadataDefineString.size() - 1);
        if (OutMetadataDefineString.back() == ',') {
            OutMetadataDefineString.back() = '\n';
        }
        OutMetadataDefineString += std::format("}};\n");
    }

    std::string ParseProperty(const clang::NamedDecl* OwnerDecl, const clang::NamedDecl* PropertyDecl, clang::QualType PropertyType, std::vector<std::string> PropertyFlags, const std::string& UnqiueStructName)
    {
        std::vector<std::string> PropertyCodes;
        std::shared_ptr<CArrayPropertyConstructor> ArrayPropertyConstructor;
        std::shared_ptr<CPtrPropertyConstructor> PtrPropertyConstructor;
        std::shared_ptr<CPropertyConstructor> PropertyConstructor;
        auto TopPropertyType = PropertyType;
        if (!PropertyType->isVoidType())
        {
            if (PropertyType->isArrayType()) {
                ArrayPropertyConstructor = std::make_shared<CArrayPropertyConstructor>();
                const clang::ArrayType* ArrayPropertyType =
                    PropertyType->getAsArrayTypeUnsafe();
                PropertyType = ArrayPropertyType->getElementType();
            }
            if (PropertyType->isPointerType()) {
                PtrPropertyConstructor = std::make_shared<CPtrPropertyConstructor>();
                PropertyType = PropertyType->getPointeeType();
            }
            else if (PropertyType->isReferenceType()) {
                if (auto OwnerFunctionDecl = dyn_cast<clang::FunctionDecl>(OwnerDecl))
                {
                    PropertyFlags.push_back("PF_ReferenceParam");
                    PtrPropertyConstructor = std::make_shared<CPtrPropertyConstructor>();
                }
                else
                {
                    throw std::runtime_error("???");
                }
                PropertyType = PropertyType->getPointeeType();
            }
            if (PropertyType->isBuiltinType()) {
                auto PropertyTypeInfo =
                    Context->getTypeInfo(PropertyType.getTypePtr());
                if (PropertyType->isSignedIntegerType()) {
                    if (PropertyTypeInfo.Width / 8 == 1)   PropertyConstructor = std::make_shared<CI8PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 2)PropertyConstructor = std::make_shared<CI16PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 4)PropertyConstructor = std::make_shared<CI32PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 8)PropertyConstructor = std::make_shared<CI64PropertyConstructor>();
                }
                else if (PropertyType->isUnsignedIntegerType()) {
                    if (PropertyTypeInfo.Width / 8 == 1)     PropertyConstructor = std::make_shared<CU8PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 2)PropertyConstructor = std::make_shared<CU16PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 4)PropertyConstructor = std::make_shared<CU32PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 8)PropertyConstructor = std::make_shared<CU64PropertyConstructor>();
                }
                else if (PropertyType->isFloatingType()) {
                    if (PropertyTypeInfo.Width / 8 == 4)      PropertyConstructor = std::make_shared<CF32PropertyConstructor>();
                    else if (PropertyTypeInfo.Width / 8 == 8) PropertyConstructor = std::make_shared<CF64PropertyConstructor>();
                }
                else if (PropertyType->isBooleanType()) {
                    PropertyConstructor = std::make_shared<CBoolPropertyConstructor>();
                }
                else {
                    throw std::runtime_error("???");
                }
            }
            else if (PropertyType->isStructureOrClassType()) {
                PropertyConstructor = std::make_shared<CClassPropertyConstructor>();
                clang::CXXRecordDecl* FinalCXXRecordDecl = PropertyType->getAsCXXRecordDecl();
                if (PropertyType.getAsString() == "std::string") {
                    PropertyConstructor = std::make_shared<CStrPropertyConstructor>();
                }
                else {
                    // 寻找
                    auto FindObjectCXXRecordDecl = FinalCXXRecordDecl;
                    while (FindObjectCXXRecordDecl) {
                        if (FindObjectCXXRecordDecl->getQualifiedNameAsString() ==
                            "RtCXX::OObject") {
                            if (PtrPropertyConstructor)
                            {
                                PtrPropertyConstructor = std::make_shared<CObjectPtrPropertyConstructor>();
                            }
                            break;
                        }
                        if (FindObjectCXXRecordDecl->getNumBases() > 0) {
                            FindObjectCXXRecordDecl = FindObjectCXXRecordDecl->bases_begin()->getType()->getAsCXXRecordDecl();
                        }
                        else {
                            FindObjectCXXRecordDecl = nullptr;
                        }
                    }
                    auto PropertyConstructorPtr = std::make_shared<CClassPropertyConstructor>();
                    PropertyConstructor = PropertyConstructorPtr;
                    PropertyConstructorPtr->ClassDecl = FinalCXXRecordDecl;
                }
            }
            else if (PropertyType->isEnumeralType()) {
                PropertyConstructor = std::make_shared<CEnumPropertyConstructor>();
            }
            else {
                throw std::runtime_error("???");
            }
        }
        else
        {
            PropertyConstructor = std::make_shared<CPropertyConstructor>();
        }
        std::shared_ptr<CPropertyConstructor> RootPropertyConstructor;
        RootPropertyConstructor = PropertyConstructor;
        if (PtrPropertyConstructor)
        {
            RootPropertyConstructor = PtrPropertyConstructor;
            PtrPropertyConstructor->PointerToProp = PropertyConstructor;
        }
        if (ArrayPropertyConstructor)
        {
            RootPropertyConstructor = ArrayPropertyConstructor;
            RootPropertyConstructor = ArrayPropertyConstructor;
            if (PtrPropertyConstructor)
            {
                ArrayPropertyConstructor->ElementProp = PtrPropertyConstructor;
            }
            else
            {
                ArrayPropertyConstructor->ElementProp = PropertyConstructor;
            }
        }
        RootPropertyConstructor->OwnerDecl = OwnerDecl;
        RootPropertyConstructor->PropertyDecl = PropertyDecl;
        if (RootPropertyConstructor == PropertyConstructor)
        {

        }
        else if (RootPropertyConstructor == PtrPropertyConstructor)
        {

        }
        else if (RootPropertyConstructor == ArrayPropertyConstructor)
        {

        }
        auto PropertyConstructorPtr = PropertyConstructor->CastTo<CClassPropertyConstructor>();
        std::string PropertyFlagsStr = MakeFlags(PropertyFlags, "PF_None");
        if (auto OwnerCXXRecordDecl = dyn_cast<clang::CXXRecordDecl>(OwnerDecl))
        {
            return std::format("CurrentProperty = StaticCreateUniqueProperty<{:s}, {:s}>(CurrentClass, \"{:s}\", {:s}, {:s}, {:s}, Controller);",
                UnqiueStructName,
                "decltype(&" + OwnerCXXRecordDecl->getQualifiedNameAsString() + "::" + PropertyDecl->getNameAsString() + ")",
                PropertyDecl->getNameAsString(),
                "offsetof(" + OwnerCXXRecordDecl->getQualifiedNameAsString() + ", " + PropertyDecl->getNameAsString() + ")",
                PropertyFlagsStr,
                PropertyConstructorPtr ? "\"" + PropertyConstructorPtr->ClassDecl->getQualifiedNameAsString() + "\"" : "nullptr");
        }
        if (auto OwnerCXXMethodDecl = dyn_cast<clang::CXXMethodDecl>(OwnerDecl))
        {
            int ParameterIndex = -1;

            const clang::ParmVarDecl* CurParmVarDecl = nullptr;
            if (PropertyDecl)
            {
                CurParmVarDecl = dyn_cast<clang::ParmVarDecl>(PropertyDecl);
            }
            return std::format("CurrentProperty = StaticCreateUniqueProperty<{:s}, {:s}>(CurrentFunction, \"{:s}\", {:s}, {:s}, {:s}, Controller);",
                UnqiueStructName,
                "TConvertToPropertyGetter<" + TopPropertyType.getAsString() + ">",
                CurParmVarDecl ? CurParmVarDecl->getNameAsString() : "",
                CurParmVarDecl ? std::to_string(CurParmVarDecl->getFunctionScopeIndex()) : std::to_string(-1),
                PropertyFlagsStr,
                PropertyConstructorPtr ? "\"" + PropertyConstructorPtr->ClassDecl->getQualifiedNameAsString() + "\"" : "nullptr");
        }
        return "***";
    }

    std::string MakeFlags(const std::vector<std::string>& Flags, const std::string& NoneFlag)
    {
        std::string FlagsStr;
        if (!Flags.empty())
        {
            FlagsStr = Flags[0];
            for (size_t i = 1; i < Flags.size(); i++)
            {
                FlagsStr += " | ";
                FlagsStr += Flags[i];
            }
            return FlagsStr;
        }
        else
        {
            return NoneFlag;
        }
    }

    virtual void onStartOfTranslationUnit() override
    {

    }

    virtual void onEndOfTranslationUnit() override {
        if (RtCXXSourceManager && Context) {
            for (auto CXXRecordDecl : CXXRecordDecls) {
                std::unordered_map<std::string, std::string> OutCXXRecordDeclMetadata;
                if (ParseReflectAnnotation(CXXRecordDecl, OutCXXRecordDeclMetadata)) {
                    WriteCodeLine(std::format("RtCXX::CMetaClass* {0:s}::GVar_StaticClass = {0:s}::StaticClass();", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("RtCXX::CMetaClass* {0:s}::StaticClass()", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("{{", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("    using namespace RtCXX;", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("    auto Controller = GetControllerPtr();", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("    static CMetaClass* ClassPtr = [&]() -> CMetaClass* {{", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("        CMetaClass* CurrentClass = nullptr;", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("        CMetaFunction* CurrentFunction = nullptr;", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("        CMetaProperty* CurrentProperty = nullptr;", CXXRecordDecl->getQualifiedNameAsString()));
                    PushIndent(8);
                    std::string BaseClassName;
                    std::vector<std::string> InterfacesName;
                    auto BaseIterator = CXXRecordDecl->bases_begin();
                    if (BaseIterator != CXXRecordDecl->bases_end())
                    {
                        auto BaseCXXRecordDecl =
                            (*BaseIterator).getType()->getAsCXXRecordDecl();
                        std::unordered_map<std::string, std::string> BaseCXXRecordDeclMetadata;
                        if (ParseReflectAnnotation(BaseCXXRecordDecl, BaseCXXRecordDeclMetadata)) {
                            BaseClassName = BaseCXXRecordDecl->getQualifiedNameAsString();
                        }
                        for (; BaseIterator != CXXRecordDecl->bases_end(); BaseIterator++) {

                        }
                    }
                    std::string InterfacesNameStr = "{ ";
                    for (size_t i = 0; i < InterfacesName.size(); i++)
                    {
                        InterfacesNameStr += "\"" + InterfacesName[i] + "\"";
                        InterfacesNameStr += ", ";
                    }
                    InterfacesNameStr += "nullptr }";
                    WriteCodeLine(std::format("CurrentClass = StaticCreateUniqueClass<{0:s}>(nullptr, \"{0:s}\", CF_None, {1:s}, {2:s}, Controller);", 
                        CXXRecordDecl->getQualifiedNameAsString(), 
                        BaseClassName.empty() ? "nullptr" : "\"" + BaseClassName + "\"",
                        InterfacesNameStr));

                    for (auto MethodIterator = CXXRecordDecl->method_begin();
                        MethodIterator != CXXRecordDecl->method_end(); MethodIterator++)
                    {
                        auto Method = *MethodIterator;
                        std::unordered_map<std::string, std::string> MethodMetadata;
                        if (ParseReflectAnnotation(Method, MethodMetadata)) {
                            WriteCodeLine("{");
                            PushIndent();
                            std::vector<std::string> FunctionFlags;
                            std::string ParamStr;
                            if (Method->isStatic()) FunctionFlags.push_back("FF_Global");
                            auto ParamIterator = Method->param_begin();
                            if (ParamIterator != Method->param_end())
                            {
                                auto Param = *ParamIterator;
                                ParamStr += Param->getType().getAsString();
                                ParamIterator++;
                                for (; ParamIterator != Method->param_end(); ParamIterator++)
                                {
                                    auto Param = *ParamIterator;
                                    ParamStr += ", ";
                                    ParamStr += Param->getType().getAsString();
                                }
                            }
                            std::string To_asSFuncPtr = "asMETHODPR("
                                + CXXRecordDecl->getQualifiedNameAsString() + ", "
                                + Method->getNameAsString() + ", "
                                + "(" + ParamStr + ")" + ", "
                                + Method->getReturnType().getAsString() + ")";
                            std::string MethodUnqiueStructName = InsertUnqiueStruct(Method);
                            WriteCodeLine(std::format("CurrentFunction = StaticCreateUniqueFunction<{:s}, {:s}>(CurrentClass, \"{:s}\", {:s}, {:s}, Controller);",
                                MethodUnqiueStructName,
                                CXXRecordDecl->getQualifiedNameAsString(),
                                Method->getNameAsString(),
                                To_asSFuncPtr,
                                MakeFlags(FunctionFlags, "FF_None")));

                            std::vector<std::string> MethodReturnPropertyFlags = { "PF_Param", "PF_OutParam", "PF_ReturnParam" };
                            WriteCodeLine(ParseProperty(Method, nullptr, Method->getReturnType(), MethodReturnPropertyFlags, InsertUnqiueStruct(Method, true)));
                            for (auto ParamIterator = Method->parameters().rbegin(); ParamIterator != Method->parameters().rend(); ParamIterator++)
                            {
                                auto Param = *ParamIterator;
                                std::vector<std::string> MethodParamPropertyFlags = { "PF_Param" };
                                WriteCodeLine(ParseProperty(Method, Param, Param->getType(), MethodParamPropertyFlags, InsertUnqiueStruct(Param)));
                            }
                            PopIndent();
                            WriteCodeLine("}");
                        }
                    }

                    for (auto FieldIterator = CXXRecordDecl->field_begin();
                        FieldIterator != CXXRecordDecl->field_end(); FieldIterator++) {
                        auto Field = *FieldIterator;
                        std::unordered_map<std::string, std::string> FieldMetadata;
                        if (ParseReflectAnnotation(Field, FieldMetadata))
                        {
                            WriteCodeLine(ParseProperty(CXXRecordDecl, Field, Field->getType(), {}, InsertUnqiueStruct(Field)));
                        }
                    }
                    PopIndent(8);
                    WriteCodeLine(std::format("        return CurrentClass;", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("    }}();", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("    return ClassPtr;", CXXRecordDecl->getQualifiedNameAsString()));
                    WriteCodeLine(std::format("}};", CXXRecordDecl->getQualifiedNameAsString()));
                }
            }
        }
        
        for (size_t i = 0; i < CodeLines.size(); i++)
        {
            RtCXXCppFileContext += CodeLines[i];
        }
        Context = nullptr;
        RtCXXSourceManager = nullptr;
    }

    std::string InsertUnqiueStruct(const clang::NamedDecl* InTarget, bool bIsFunctionReturnValue = false)
    {
        std::string OwnerName;
        std::string TargetNmae;
        std::string UnqiueAnonymousStructName;

        const clang::CXXRecordDecl* CastCXXRecordDecl = nullptr;
        const clang::CXXMethodDecl* CastCXXMethodDecl = nullptr;
        const clang::ParmVarDecl* CastParmVarDecl = nullptr;
        const clang::FieldDecl* CastFieldDecl = nullptr;
        if (bIsFunctionReturnValue)
        {
            UnqiueAnonymousStructName += "FR"; // FuncRet
            if (auto CastFunctionDecl = dyn_cast<clang::FunctionDecl>(InTarget))
            {
                UnqiueAnonymousStructName += "____";
                TargetNmae = CastFunctionDecl->getQualifiedNameAsString();
                std::transform(TargetNmae.cbegin(), TargetNmae.cend(), TargetNmae.begin(), [](unsigned char c) { return c == ':' ? '_' : c; });
                UnqiueAnonymousStructName += TargetNmae;
                UnqiueAnonymousStructName += "__Return";
                CodeLines.insert(CodeLines.begin(), std::format("struct {0:s} {{}};\n", UnqiueAnonymousStructName));
                return UnqiueAnonymousStructName;
            }
            else
            {
                throw std::runtime_error("???");
            }
        }
        else if (CastCXXRecordDecl = dyn_cast<clang::CXXRecordDecl>(InTarget))
        {
            UnqiueAnonymousStructName += "GC";
        }
        else if (CastCXXMethodDecl = dyn_cast<clang::CXXMethodDecl>(InTarget))
        {
            UnqiueAnonymousStructName += "CF";
        }
        else if (CastParmVarDecl = dyn_cast<clang::ParmVarDecl>(InTarget))
        {
            UnqiueAnonymousStructName += "FP";
        }
        else if (CastFieldDecl = dyn_cast<clang::FieldDecl>(InTarget))
        {
            UnqiueAnonymousStructName += "GP";

        }
        else
            throw std::runtime_error("???");
        UnqiueAnonymousStructName += "____";

        if (CastParmVarDecl)
        {
            const clang::FunctionDecl* ThisFunctionDecl = static_cast<const clang::FunctionDecl*>(CastParmVarDecl->getParentFunctionOrMethod());
            TargetNmae = ThisFunctionDecl->getQualifiedNameAsString();
        }
        else
        {
            TargetNmae = InTarget->getQualifiedNameAsString();
        }
        std::transform(TargetNmae.cbegin(), TargetNmae.cend(), TargetNmae.begin(), [](unsigned char c) { return c == ':' ? '_' : c; });
        UnqiueAnonymousStructName += TargetNmae;

        if (CastParmVarDecl)
        {
            UnqiueAnonymousStructName += "__Param" + std::to_string(CastParmVarDecl->getFunctionScopeIndex());
        }
        CodeLines.insert(CodeLines.begin(), std::format("struct {0:s} {{}};\n", UnqiueAnonymousStructName));
        return UnqiueAnonymousStructName;
    }

    void PushIndent(int InIndentWidth = 4)
    {
        for (size_t i = 0; i < InIndentWidth; i++)
        {
            CurrentIndentWidth.push_back(' ');
        }
    }
    void WriteCodeLine(const std::string& InCodeLine)
    {
        CodeLines.push_back(CurrentIndentWidth + InCodeLine + "\n");
    }
    void PopIndent(int InIndentWidth = 4)
    {
        for (size_t i = 0; i < InIndentWidth; i++)
        {
            CurrentIndentWidth.pop_back();
        }
    }

    std::string CurrentIndentWidth;
    std::vector<std::string> CodeLines;

    std::unordered_set<clang::CXXRecordDecl const*> CXXRecordDecls;
    std::unordered_map<clang::FunctionDecl const*, clang::CXXRecordDecl const*> FunctionDeclMap;
    std::unordered_map<clang::FieldDecl const*, clang::CXXRecordDecl const*>FieldDeclMap;

    std::vector<clang::EnumDecl const*> EnumDecls;
    clang::ASTContext* Context{ nullptr };
    clang::SourceManager* RtCXXSourceManager{ nullptr };
};
