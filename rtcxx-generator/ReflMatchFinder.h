#pragma once
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Lex/Lexer.h>
#include <filesystem>
#include <fstream>
#include <any>
#include <regex>

extern std::unordered_set<std::string> HeaderFiles;
extern std::string RtCXXCppFileContext;
static std::unordered_set<std::string> RtCXXMetadataTypeSet = { "Class", "Struct", "Enum", "Property", "Function" };

bool SplitAttrStr(const std::string& AttrStr, std::vector<std::pair<std::string, std::string>>& KVStrs, int32_t MaxParsedMetadataCounter);

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
    bool ParseReflectAnnotation(const clang::Decl* CheckedDecl, std::string& OutRtCXXMetadataType, std::vector<std::pair<std::string, std::string>>& OutMetadatas, int32_t MaxParsedMetadataCounter = INT32_MAX) {
        if (CheckedDecl->hasAttrs()) {
            for (auto Attr : CheckedDecl->getAttrs()) {
                if (Attr->getKind() == clang::attr::Annotate) {
                    std::string RowStringBuffer;
                    llvm::raw_string_ostream RowStringOutputStream(RowStringBuffer);
                    Attr->printPretty(RowStringOutputStream, clang::PrintingPolicy(clang::LangOptions()));
                    std::string AttrString(RowStringOutputStream.str());
                    // 移除前部 `[[clang::annotate("`
                    bool bNotIsRtCXXAttr = false;
                    int Index = 0;
                    const char* Left = "[[clang::annotate(\"";
                    while (*Left != '\0')
                    {
                        if (AttrString.size() > Index)
                        {
                            if (AttrString[Index] == ' ')
                                Index++;
                            else if (AttrString[Index] == *Left)
                                Index++, Left++;
                            else
                            {
                                bNotIsRtCXXAttr = true;
                                break;
                            }
                        }
                        else
                        {
                            bNotIsRtCXXAttr = true;
                            break;
                        }
                    }
                    if (bNotIsRtCXXAttr) continue;
                    AttrString = AttrString.substr(Index);
                    // 移除后部 `")]]`
                    Index = AttrString.size() - 1;
                    const char* Right = "\0\")]]";
                    Right++;
                    while (*Right != '\0') Right++;
                    Right--;
                    while (*Right != '\0')
                    {
                        if (AttrString.size() > Index)
                        {
                            if (AttrString[Index] == ' ')
                                Index--;
                            else if (AttrString[Index] == *Right)
                                Index--, Right--;
                            else
                            {
                                bNotIsRtCXXAttr = true;
                                break;
                            }
                        }
                        else
                        {
                            bNotIsRtCXXAttr = true;
                            break;
                        }
                    }
                    if (bNotIsRtCXXAttr) continue;
                    AttrString = AttrString.substr(0, Index + 1);
                    std::string FindRtCXXMetadataType;
                    Index = 0;
                    while (AttrString.size() > Index)
                    {
                        if (AttrString[Index] == ',')
                        {
                            Index++;
                            break;
                        }
                        FindRtCXXMetadataType.push_back(AttrString[Index]);
                        Index++;
                    }
                    AttrString = AttrString.substr(Index);
                    if (RtCXXMetadataTypeSet.contains(FindRtCXXMetadataType))
                    {
                        OutRtCXXMetadataType = FindRtCXXMetadataType;
                        if (MaxParsedMetadataCounter > 0)
                        {
                            SplitAttrStr(AttrString, OutMetadatas, MaxParsedMetadataCounter);
                        }
                        return true;
                    }
                }
            }
        }
        return false;
    }

    std::string ParseProperty(const clang::NamedDecl* OwnerDecl, const clang::NamedDecl* PropertyDecl, clang::QualType PropertyType, std::vector<std::string> PropertyFlags, const std::string& UnqiueStructName)
    {
        std::vector<std::string> PropertyCodes;
        const clang::CXXRecordDecl* ClassDecl = nullptr;
        auto TopPropertyType = PropertyType;
        if (!PropertyType->isVoidType())
        {
            if (PropertyType->isArrayType()) {
                const clang::ArrayType* ArrayPropertyType = PropertyType->getAsArrayTypeUnsafe();
                PropertyType = ArrayPropertyType->getElementType();
            }
            if (PropertyType->isPointerType()) {
                PropertyType = PropertyType->getPointeeType();
            }
            else if (PropertyType->isReferenceType()) {
                if (auto OwnerFunctionDecl = dyn_cast<clang::FunctionDecl>(OwnerDecl))
                {
                    PropertyFlags.push_back("PF_ReferenceParam");
                }
                else
                {
                    throw std::runtime_error("???");
                }
                PropertyType = PropertyType->getPointeeType();
            }
            if (PropertyType->isBuiltinType()) {

            }
            else if (PropertyType->isStructureOrClassType()) {
                clang::CXXRecordDecl* FinalCXXRecordDecl = PropertyType->getAsCXXRecordDecl();
                if (PropertyType.getAsString() == "std::string") {
                }
                else {
                    //// 寻找
                    //auto FindObjectCXXRecordDecl = FinalCXXRecordDecl;
                    //while (FindObjectCXXRecordDecl) {
                    //    if (FindObjectCXXRecordDecl->getQualifiedNameAsString() == "RtCXX::OObject") {
                    //        break;
                    //    }
                    //    if (FindObjectCXXRecordDecl->getNumBases() > 0) {
                    //        FindObjectCXXRecordDecl = FindObjectCXXRecordDecl->bases_begin()->getType()->getAsCXXRecordDecl();
                    //    }
                    //    else {
                    //        FindObjectCXXRecordDecl = nullptr;
                    //    }
                    //}
                    ClassDecl = FinalCXXRecordDecl;
                }
            }
            else if (PropertyType->isEnumeralType()) {
            }
            else {
                throw std::runtime_error("???");
            }
        }
        else
        {

        }
        std::string PropertyFlagsStr = MakeFlags(PropertyFlags, "PF_None");
        if (auto OwnerCXXRecordDecl = dyn_cast<clang::CXXRecordDecl>(OwnerDecl))
        {
            return std::format("CurrentProperty = StaticCreateUniqueProperty<{:s}, {:s}>(CurrentClass, \"{:s}\", {:s}, {:s}, {:s}, Controller);",
                UnqiueStructName,
                "decltype(&" + OwnerCXXRecordDecl->getQualifiedNameAsString() + "::" + PropertyDecl->getNameAsString() + ")",
                PropertyDecl->getNameAsString(),
                "offsetof(" + OwnerCXXRecordDecl->getQualifiedNameAsString() + ", " + PropertyDecl->getNameAsString() + ")",
                PropertyFlagsStr,
                ClassDecl ? "\"" + ClassDecl->getQualifiedNameAsString() + "\"" : "nullptr");
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
                ClassDecl ? "\"" + ClassDecl->getQualifiedNameAsString() + "\"" : "nullptr");
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
                std::string CXXRecordDeclMetaTag;
                std::vector<std::pair<std::string, std::string>> CXXRecordDeclMetadata;
                if (ParseReflectAnnotation(CXXRecordDecl, CXXRecordDeclMetaTag, CXXRecordDeclMetadata)) {
                    bool bIsClass = CXXRecordDeclMetaTag == "Class";
                    bool bIsStruct = CXXRecordDeclMetaTag == "Struct";
                    //bool bIsEnum = RtCXXMetadataType == "Enum";
                    WriteCodeLine(std::format("RtCXX::CMeta{1:s}* {0:s}::GVar_Static{1:s} = {0:s}::Static{1:s}();", CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("RtCXX::CMeta{1:s}* {0:s}::Static{1:s}()"                           , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("{{"                                                                , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("    using namespace RtCXX;"                                        , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("    auto Controller = GetControllerPtr();"                         , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("    static CMeta{1:s}* ClassPtr = [&]() -> CMeta{1:s}* {{"         , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("        CMeta{1:s}* Current{1:s} = nullptr;"                       , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("        CMetaProperty* CurrentProperty = nullptr;"                 , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    WriteCodeLine(std::format("        CMetaFunction* CurrentFunction = nullptr;"                 , CXXRecordDecl->getQualifiedNameAsString(), CXXRecordDeclMetaTag));
                    PushIndent(8);
                    if (bIsClass)
                    {
                        std::string BaseClassName;
                        std::vector<std::string> InterfacesName;
                        auto BaseIterator = CXXRecordDecl->bases_begin();
                        if (BaseIterator != CXXRecordDecl->bases_end())
                        {
                            auto BaseCXXRecordDecl = (*BaseIterator).getType()->getAsCXXRecordDecl();
                            std::string BaseCXXRecordDeclMetaTag;
                            std::vector<std::pair<std::string, std::string>> BaseCXXRecordDeclMetadata;
                            if (ParseReflectAnnotation(BaseCXXRecordDecl, BaseCXXRecordDeclMetaTag, BaseCXXRecordDeclMetadata, 0)) {
                                if (CXXRecordDeclMetaTag == "Class")
                                {
                                    BaseClassName = BaseCXXRecordDecl->getQualifiedNameAsString();
                                }
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
                            std::string MethodMetaTag;
                            std::vector<std::pair<std::string, std::string>> MethodMetadata;
                            if (ParseReflectAnnotation(Method, MethodMetaTag, MethodMetadata) && MethodMetaTag == "Function") {
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
                    }
                    for (auto FieldIterator = CXXRecordDecl->field_begin();
                        FieldIterator != CXXRecordDecl->field_end(); FieldIterator++) {
                        auto Field = *FieldIterator;
                        std::string FieldMetaTag;
                        std::vector<std::pair<std::string, std::string>> FieldMetadata;
                        if (ParseReflectAnnotation(Field, FieldMetaTag, FieldMetadata) && FieldMetaTag == "Property")
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


bool SplitAttrStr(const std::string& AttrStr, std::vector<std::pair<std::string, std::string>>& KVStrs, int32_t MaxParsedMetadataCounter)
{
    int Index = 0;
    enum EWhatToFind
    {
        WTF_Key,
        WTF_Equal,
        WTF_Value,
        WTF_NextKV,
    } WhatToFind = WTF_Key;
    bool bDontIgnoreSpace = false;
    bool bIsStringValue = false;
    std::string Key;
    std::string Value;
    int32_t CurrentParsedMetadataCounter = 0;
    for (size_t i = 0; i < AttrStr.size(); i++)
    {
        if (CurrentParsedMetadataCounter >= MaxParsedMetadataCounter) return true;
        if (AttrStr[i] == ' ' && !bDontIgnoreSpace)
        {
            continue;
        }
        if (WhatToFind == WTF_Key)
        {
            if (AttrStr[i] == '=' || AttrStr[i] == ',')
            {
                if (!std::regex_match(Key, std::regex("^[_a-zA-Z][_a-zA-Z0-9]*$")))
                {
                    throw std::runtime_error("???");
                }
                if (AttrStr[i] == '=')
                {
                    WhatToFind = WTF_Value;
                }
                else
                {
                    KVStrs.push_back(std::pair(Key, "std::any()"));
                    bDontIgnoreSpace = false;
                    bIsStringValue = false;
                    Key.clear();
                    Value.clear();
                    CurrentParsedMetadataCounter++;
                }
                continue;
            }
            Key.push_back(AttrStr[i]);
        }
        else if (WhatToFind == WTF_Value)
        {
            if (Value.empty())
            {
                if (AttrStr[i] == '\"')
                {
                    bIsStringValue = true;
                }
                bDontIgnoreSpace = true;
                Value.push_back(AttrStr[i]);
            }
            else
            {
                if (bIsStringValue)
                {
                    if (AttrStr[i] == '\\')
                    {
                        Value.push_back(AttrStr[i]);
                        if (AttrStr.size() > i + 1)
                        {
                            i++;
                            Value.push_back(AttrStr[i]);
                            continue;
                        }
                        else
                        {
                            throw std::runtime_error("???");
                        }
                    }
                    if (AttrStr[i] == '\"')
                    {
                        Value.push_back(AttrStr[i]);
                        KVStrs.push_back(std::pair(Key, Value));
                        bDontIgnoreSpace = false;
                        bIsStringValue = false;
                        Key.clear();
                        Value.clear();
                        CurrentParsedMetadataCounter++;
                        i++;
                        while (AttrStr.size() > i)
                        {
                            if (AttrStr[i] == ',')
                            {
                                WhatToFind = WTF_Key;
                                break;
                            }
                            else if (' ')
                            {
                                i++;
                            }
                            else
                            {
                                throw std::runtime_error("???");
                            }
                        }
                        continue;
                    }
                }
                else
                {
                    if (AttrStr[i] == ',')
                    {
                        while (Value.back() == ' ') Value.pop_back();
                        KVStrs.push_back(std::pair(Key, Value));
                        bDontIgnoreSpace = false;
                        bIsStringValue = false;
                        Key.clear();
                        Value.clear();
                        CurrentParsedMetadataCounter++;
                        WhatToFind = WTF_Key;
                        continue;
                    }
                }
                Value.push_back(AttrStr[i]);
            }
        }
        else
            throw std::runtime_error("???");
    }
    if (bIsStringValue)
    {
        throw std::runtime_error("???");
    }
    if (!Key.empty() )
    {
        if (Value.empty())
        {
            KVStrs.push_back(std::pair(Key, "std::any()"));
        }
        else
        {
            KVStrs.push_back(std::pair(Key, Value));
        }
    }
    return true;
}
