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

  std::string GetPropertyName() 
  {
      std::string PropertyName = "____";
      if (OwnerDecl)
      {
          PropertyName.append(OwnerDecl->getNameAsString());
      }
      PropertyName.append("__");
      if (PropertyDecl)
      {
          PropertyName.append(PropertyDecl->getNameAsString());
      }
      else
      {
          PropertyName.append("_");
      }
      return PropertyName;
  }

  virtual void ConstructCode(std::vector<std::string>& Lines) {
    std::string Code;
    if (auto OwnerCXXRecordDecl = dyn_cast<clang::CXXRecordDecl>(OwnerDecl))
    {
        Lines.push_back(std::format("		static {2:s} {1:s}(\"{3:s}\", offsetof({0:s}, {3:s}));\n", OwnerDecl->getNameAsString(), GetPropertyName(), GetPropertyType(), PropertyDecl->getNameAsString()));
        Lines.push_back(std::format("		{0:s}.SetOwner(&Class);\n", GetPropertyName()));
        Lines.push_back(std::format("		Class.InsertProperty(&{0:s});\n", GetPropertyName()));
    }
    if (auto OwnerFunctionDecl = dyn_cast<clang::FunctionDecl>(OwnerDecl))
    {
        Lines.push_back(std::format("		static {2:s} {1:s}(\"{0:s}\", 0);\n", PropertyDecl ? PropertyDecl->getNameAsString() : "_", GetPropertyName(), GetPropertyType()));
        Lines.push_back(std::format("		{0:s}.SetOwner(&____{1:s});\n", GetPropertyName(), OwnerFunctionDecl->getNameAsString()));
        Lines.push_back(std::format("		____{1:s}.InsertProperty(&{0:s});\n", GetPropertyName(), OwnerFunctionDecl->getNameAsString()));
        if (!PropertyDecl)
        {
            Lines.push_back(std::format("		{0:s}.bIsReturn = true;\n", GetPropertyName()));
        }
        else
        {
            Lines.push_back(std::format("		{0:s}.bIsReturn = false;\n", GetPropertyName()));
        }
    }
    if (OwnerDecl)
    {
        Lines.push_back(std::format("		{0:s}.bIsStatic = false;\n", GetPropertyName()));
    }
    Lines.push_back(std::format("		Controller->RegisterMetadata(&{0:s});\n", GetPropertyName()));
  }

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
    std::string ClassName;
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
    virtual void ConstructCode(std::vector<std::string>& Lines) {
        CPropertyConstructor::ConstructCode(Lines);
        if (auto PointerToClassProp = PointerToProp->CastTo<CClassPropertyConstructor>())
        {
            Lines.push_back(std::format("        Controller->GetClassByAfterRegisterClassCallback(\"{0:s}\", [&](CMetaClass* InMetaClass) {{ {1:s}.PointerToProp = InMetaClass->StandardProperty; }});\n", PointerToClassProp->ClassName, GetPropertyName()));
        }
        else if (auto PointerToClassProp = PointerToProp->CastTo<CEnumPropertyConstructor>())
        {
            //                Code += std::format(
            //R"(
            //        Controller->GetClassByAfterRegisterClassCallback("{0:s}", [&](CMetaClass* InMetaClass) {{
            //            {1:s}.PointerToProp = InMetaClass->StandardPtr{2:s}; 
            //        }})",
            //                    PointerToClassProp->ClassName, GetPropertyName(), PropertyType);
        }
        else
        {
            Lines.push_back(std::format("        {0:s}.PointerToProp = &Standard{1:s};\n", GetPropertyName(), PointerToProp->GetPropertyType()));
        }
    };
};

struct CObjectPtrPropertyConstructor : public CPtrPropertyConstructor {
    DECL_CAST(CObjectPtrPropertyConstructor, CPtrPropertyConstructor)
    virtual std::string GetPropertyType() { return "CObjectPtrProperty"; }
};

struct CArrayPropertyConstructor : public CPropertyConstructor {
    DECL_CAST(CArrayPropertyConstructor, CPropertyConstructor)
    virtual std::string GetPropertyType() { return "CArrayProperty"; }
    virtual void ConstructCode(std::vector<std::string>& Lines) {
         CPropertyConstructor::ConstructCode(Lines);
        std::string PtrStr;
        auto PropConstructor = ElementProp.get();
        if (auto ElementPtrProp = PropConstructor->CastTo<CPtrPropertyConstructor>())
        {
            PtrStr = "Ptr";
            PropConstructor = ElementPtrProp->PointerToProp.get();
        }
        if (auto ElementClassProp = PropConstructor->CastTo<CClassPropertyConstructor>())
        {
            Lines.push_back(std::format("Controller->GetClassByAfterRegisterClassCallback(\"{0:s}\", [&](CMetaClass* InMetaClass) {{ {1:s}.ElementProp = InMetaClass->Standard{2:s}; }});\n",
                ElementClassProp->ClassName, GetPropertyName(), PtrStr));
        }
        else if (auto PointerToClassProp = PropConstructor->CastTo<CEnumPropertyConstructor>())
        {
            //                Code += std::format(
            //R"(
            //        Controller->GetClassByAfterRegisterClassCallback("{0:s}", [&](CMetaClass* InMetaClass) {{
            //            {1:s}.PointerToProp = InMetaClass->StandardPtr{2:s}; 
            //        }})",
            //                    PointerToClassProp->ClassName, GetPropertyName(), PropertyType);
        }
        else
        {
            Lines.push_back(std::format("{0:s}.ElementProp = &Standard{1:s};\n", GetPropertyName(), PtrStr, PropConstructor->GetPropertyType()));
        }
    }
    std::shared_ptr<CPropertyConstructor> ElementProp;
};

class ReflClassMatchFinder
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  void run(const clang::ast_matchers::MatchFinder::MatchResult &MatchResult) {
    Context = MatchResult.Context;
    RtCXXSourceManager = MatchResult.SourceManager;

    clang::Decl const *Decl = MatchResult.Nodes.getNodeAs<clang::Decl>("Decl");
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
      if (clang::EnumDecl const *EnumDecl =
              MatchResult.Nodes.getNodeAs<clang::EnumDecl>("Decl")) {
        EnumDecls.push_back(EnumDecl);
      }

      if (clang::CXXRecordDecl const *CXXRecordDecl =
              MatchResult.Nodes.getNodeAs<clang::CXXRecordDecl>("Decl")) {
        CXXRecordDecls.insert(CXXRecordDecl);
      }

      if (clang::FieldDecl const *FieldDecl =
              MatchResult.Nodes.getNodeAs<clang::FieldDecl>("Decl")) {
        clang::CXXRecordDecl const *Parent =
            cast<clang::CXXRecordDecl const>(FieldDecl->getParent());
        FieldDeclMap.insert(std::make_pair(FieldDecl, Parent));
      }

      if (clang::FunctionDecl const *FunctionDecl =
              MatchResult.Nodes.getNodeAs<clang::FunctionDecl>("Decl")) {
        clang::CXXRecordDecl const *Parent =
            cast<clang::CXXRecordDecl const>(FunctionDecl->getParent());
        FunctionDeclMap.insert(std::make_pair(FunctionDecl, Parent));
      }
    }
  }

  bool ParseReflectAnnotation(
      const clang::Decl *CheckedDecl,
      std::unordered_map<std::string, std::string> &OutMetadata) {
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
                      } else {
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
                } else {
                  if (std::isdigit(AttrFullString[OffsetIndex])) {
                    while (OffsetIndex < LastIndex &&
                           (std::isdigit(AttrFullString[OffsetIndex]) ||
                            AttrFullString[OffsetIndex] != '.')) {
                      MetadataValue.push_back(AttrFullString[OffsetIndex]);
                      OffsetIndex++;
                    }
                  } else {
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
                } else if (OffsetIndex == LastIndex) {

                } else {
                  throw std::runtime_error(std::format(
                      "{:s} illegal metadata macro, correct format should be "
                      "RMETADATA(A=B, C = \"D\")",
                      Attr->getLocation().printToString(*RtCXXSourceManager)));
                }
              } else if (AttrFullString[OffsetIndex] == ',') {
                if (OutMetadata.contains(MetadataKey)) {
                  throw std::runtime_error(std::format(
                      "{:s} illegal metadata macro, correct format should be "
                      "RMETADATA(A=B, C = \"D\")",
                      Attr->getLocation().printToString(*RtCXXSourceManager)));
                }
                OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
                OffsetIndex++;
              } else if (OffsetIndex == LastIndex) {
                if (OutMetadata.contains(MetadataKey)) {
                  throw std::runtime_error(std::format(
                      "{:s} illegal metadata macro, correct format should be "
                      "RMETADATA(A=B, C = \"D\")",
                      Attr->getLocation().printToString(*RtCXXSourceManager)));
                }
                OutMetadata.insert(std::make_pair(MetadataKey, MetadataValue));
              } else {
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
  GetMetadataString(const std::string &OutDeclName,
                    const std::map<std::string, std::string> &InMetadataMap,
                    std::string &OutMetadataDefineString) {
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

  std::vector<std::string> ParseProperty(const clang::NamedDecl* OwnerDecl, const clang::NamedDecl* PropertyDecl, clang::QualType PropertyType)
  {
      std::vector<std::string> PropertyCodes;
      std::shared_ptr<CArrayPropertyConstructor> ArrayPropertyConstructor;
      std::shared_ptr<CPtrPropertyConstructor> PtrPropertyConstructor;
      std::shared_ptr<CPropertyConstructor> PropertyConstructor;
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
                  // Ptr == Ref
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
              clang::CXXRecordDecl* FieldCXXRecordDecl = PropertyType->getAsCXXRecordDecl();
              if (PropertyType.getAsString() == "std::string") {
                  PropertyConstructor = std::make_shared<CStrPropertyConstructor>();
              }
              else {
                  // 寻找
                  auto FindObjectCXXRecordDecl = FieldCXXRecordDecl;
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
                          FindObjectCXXRecordDecl =
                              FindObjectCXXRecordDecl->bases_begin()
                              ->getType()
                              ->getAsCXXRecordDecl();
                      }
                      else {
                          FindObjectCXXRecordDecl = nullptr;
                      }
                  }
                  if (!PropertyConstructor)
                  {
                      PropertyConstructor = std::make_shared<CClassPropertyConstructor>();
                  }
                  if (auto ClassPropertyConstructor = PropertyConstructor->CastTo<CClassPropertyConstructor>())
                  {
                      ClassPropertyConstructor->ClassName = FieldCXXRecordDecl->getQualifiedNameAsString();
                  }
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
      RootPropertyConstructor->ConstructCode(PropertyCodes);
      return PropertyCodes;
  }

  virtual void onStartOfTranslationUnit() override 
  {

  }

  virtual void onEndOfTranslationUnit() override {
      if (RtCXXSourceManager && Context) {
          for (auto CXXRecordDecl : CXXRecordDecls) {
              std::unordered_map<std::string, std::string> OutCXXRecordDeclMetadata;
              if (ParseReflectAnnotation(CXXRecordDecl, OutCXXRecordDeclMetadata)) {
                  std::vector<std::string> Contexts;
                  auto BaseIterator = CXXRecordDecl->bases_begin();
                  if (BaseIterator != CXXRecordDecl->bases_end())
                  {
                      auto BaseCXXRecordDecl =
                          (*BaseIterator).getType()->getAsCXXRecordDecl();
                      std::unordered_map<std::string, std::string> BaseCXXRecordDeclMetadata;
                      if (ParseReflectAnnotation(BaseCXXRecordDecl, BaseCXXRecordDeclMetadata)) {
                          std::string BaseDeclContext =
                              std::format("        Class.ExtendsClass = {0:s}::StaticClass();\n", BaseCXXRecordDecl->getQualifiedNameAsString());
                          Contexts.push_back(BaseDeclContext);
                      }
                  }
                  for (; BaseIterator != CXXRecordDecl->bases_end(); BaseIterator++) {
                  }

                  for (auto MethodIterator = CXXRecordDecl->method_begin();
                      MethodIterator != CXXRecordDecl->method_end(); MethodIterator++)
                  {
                      auto Method = *MethodIterator;
                    std::unordered_map<std::string, std::string> MethodMetadata;
                    if (ParseReflectAnnotation(Method, MethodMetadata)) {
                        Contexts.push_back(std::format("        static CMetaFunction ____{0:s}(\"{0:s}\");\n", Method->getNameAsString()));
                        Contexts.push_back(std::format("		____{0:s}.SetOwner(&Class);\n", Method->getNameAsString()));
                        Contexts.push_back(std::format("		Class.InsertFunction(&____{0:s});\n", Method->getNameAsString()));
                        Contexts.push_back(std::format("        ____{0:s}.bIsStatic = {1:s};\n", Method->getNameAsString(), Method->isStatic() ? "true" : "false"));
                       // asSMethodPtr<sizeof(void (c::*)())>::Convert(AS_METHOD_AMBIGUITY_CAST(r (c::*)p)(&c::m))
                        if (Method->isStatic())
                            Contexts.push_back(std::format("        ____{0:s}.FuncPtr = ;\n", Method->getNameAsString(), ""));
                        else
                        {
                            Method
                            std::string ParamStr;
                            auto ParamIterator = Method->param_begin();
                            if (ParamIterator != Method->param_end())
                            {
                                auto Param = *ParamIterator;
                                ParamStr += Param->getType().getAsString();
                                ParamIterator++;
                            }
                            for (; ParamIterator != Method->param_end(); ParamIterator++)
                            {
                                auto Param = *ParamIterator;
                                ParamStr += ", ";
                                ParamStr += Param->getType().getAsString();
                            }
                            std::string SignatureStr = std::format("asMETHODPR({0:s}, {1:s}, ({2:s}), {3:s})",
                                Method->getParent()->getQualifiedNameAsString(),
                                Method->getNameAsString(),
                                ParamStr,
                                Method->getReturnType().getAsString());
                            Contexts.push_back(std::format("        ____{0:s}.FuncPtr = {1:s};\n", Method->getNameAsString(), SignatureStr));
                        }



                        std::vector<std::string> CodeLines = ParseProperty(Method, nullptr, Method->getReturnType());
                        Contexts.insert(Contexts.end(), CodeLines.begin(), CodeLines.end());
                        for (auto ParamIterator = Method->param_begin(); ParamIterator != Method->param_end(); ParamIterator++)
                        {
                            auto Param = *ParamIterator;
                            CodeLines = ParseProperty(Method, Param, Param->getType());
                            Contexts.insert(Contexts.end(), CodeLines.begin(), CodeLines.end());
                        }
                        Contexts.push_back(std::format("		Controller->RegisterMetadata(&____{0:s});\n", Method->getNameAsString()));
                        Contexts.push_back(std::format("\n"));
                    }
                  }

                  for (auto FieldIterator = CXXRecordDecl->field_begin();
                      FieldIterator != CXXRecordDecl->field_end(); FieldIterator++) {
                      auto Field = *FieldIterator;
                      std::unordered_map<std::string, std::string> FieldMetadata;
                      if (ParseReflectAnnotation(Field, FieldMetadata)) 
                      {
                          std::string PropertyDeclContext;
                          clang::QualType FieldType = Field->getType();
                          std::shared_ptr<CArrayPropertyConstructor> ArrayPropertyConstructor;
                          auto PropCodes = ParseProperty(CXXRecordDecl, Field, Field->getType());
                          Contexts.insert(Contexts.end(), PropCodes.begin(), PropCodes.end());
                          Contexts.push_back(std::format("\n"));
                      }
                  }
                  std::string ContextsBuffer;
                  for (size_t i = 0; i < Contexts.size(); i++) {
                      ContextsBuffer.append(Contexts[i]);
                  }
                  RtCXXCppFileContext += std::format(
                      R"(
RtCXX::CMetaClass* {0:s}::SelfClass = {0:s}::StaticClass(); 
RtCXX::CMetaClass* {0:s}::StaticClass()                     
{{
    using namespace RtCXX;
    auto Controller = GetControllerPtr();
	static CMetaClass* ClassPtr = [&]() -> CMetaClass* {{
		static TClass<{0:s}> Class("{0:s}", Controller);

{1:s}
        Controller->RegisterMetadata(&Class);
		return &Class;
	}}();
    return ClassPtr;
}};
)",
CXXRecordDecl->getQualifiedNameAsString(), ContextsBuffer);
              }
          }
      }
      Context = nullptr;
      RtCXXSourceManager = nullptr;
  }

  std::unordered_set<clang::CXXRecordDecl const *> CXXRecordDecls;
  std::unordered_map<clang::FunctionDecl const *, clang::CXXRecordDecl const *>
      FunctionDeclMap;
  std::unordered_map<clang::FieldDecl const *, clang::CXXRecordDecl const *>
      FieldDeclMap;

  std::vector<clang::EnumDecl const *> EnumDecls;
  clang::ASTContext *Context{nullptr};
  clang::SourceManager *RtCXXSourceManager{nullptr};
};
