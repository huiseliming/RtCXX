#pragram once
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
    CI_CProperty         = 0x00000001,
    CI_CBoolProperty     = 0x00000002,
    CI_CI8Property       = 0x00000004,
    CI_CI16Property      = 0x00000008,
    CI_CI32Property      = 0x00000010,
    CI_CI64Property      = 0x00000020,
    CI_CU8Property       = 0x00000040,
    CI_CU16Property      = 0x00000080,
    CI_CU32Property      = 0x00000100,
    CI_CU64Property      = 0x00000200,
    CI_CF32Property      = 0x00000400,
    CI_CF64Property      = 0x00000800,
    CI_CEnumProperty     = 0x00001000,
    CI_CClassProperty    = 0x00002000,
    CI_CStrProperty      = 0x00004000,
    CI_CPtrProperty      = 0x00008000,
    CI_CObjectPtrProperty= 0x00010000,
    CI_CArrayProperty    = 0x00020000,
};

struct CProperty {
    constexpr static ECastId StaticECastId() {
        return ECastId(CI_CProperty);
    }
    ECastId CastId;
    CProperty()
        : CastId(CI_None)
    {
        CastId = ECastId(CastId | StaticECastId());
    }
    CProperty(ECastId InCastId)
        :CastId(CI_CProperty)
    {}

    template<typename T>
    T* CastTo()
    {
        if (CastId & T::StaticECastId())
            return static_cast<T*>(this);
        return nullptr;
    }

  virtual ~CProperty() {}
  virtual std::string GetPropertyType() { return "CMetaProperty"; }

  const clang::NamedDecl* OwnerDecl;
  const clang::NamedDecl* PropertyDecl;
};

struct CBoolProperty : public CProperty {
    DECL_CAST(CBoolProperty, CProperty)
  virtual std::string GetPropertyType() { return "CBoolProperty"; }
};

struct CI8Property : public CProperty {
    DECL_CAST(CI8Property, CProperty)
  virtual std::string GetPropertyType() { return "CI8Property"; }
};
struct CI16Property : public CProperty {
    DECL_CAST(CI16Property, CProperty)
  virtual std::string GetPropertyType() { return "CI16Property"; }
};
struct CI32Property : public CProperty {
    DECL_CAST(CI32Property, CProperty)
  virtual std::string GetPropertyType() { return "CI32Property"; }
};
struct CI64Property : public CProperty {
    DECL_CAST(CI64Property, CProperty)
  virtual std::string GetPropertyType() { return "CI64Property"; }
};
struct CU8Property : public CProperty {
    DECL_CAST(CU8Property, CProperty)
  virtual std::string GetPropertyType() { return "CU8Property"; }
};
struct CU16Property : public CProperty {
    DECL_CAST(CU16Property, CProperty)
  virtual std::string GetPropertyType() { return "CU16Property"; }
};
struct CU32Property : public CProperty {
    DECL_CAST(CU32Property, CProperty)
  virtual std::string GetPropertyType() { return "CU32Property"; }
};
struct CU64Property : public CProperty {
    DECL_CAST(CU64Property, CProperty)
  virtual std::string GetPropertyType() { return "CU64Property"; }
};
struct CF32Property : public CProperty {
    DECL_CAST(CF32Property, CProperty)
  virtual std::string GetPropertyType() { return "CF32Property"; }
};
struct CF64Property : public CProperty {
    DECL_CAST(CF64Property, CProperty)
  virtual std::string GetPropertyType() { return "CF64Property"; }
};
struct CClassProperty : public CProperty {
    DECL_CAST(CClassProperty, CProperty)
    virtual std::string GetPropertyType() { return "CClassProperty"; }
    clang::CXXRecordDecl* ClassDecl;
};
struct CEnumProperty : public CProperty {
    DECL_CAST(CEnumProperty, CProperty)
    virtual std::string GetPropertyType() { return "CEnumProperty"; }
    std::string EnumName;
};
struct CStrProperty : public CProperty {
    DECL_CAST(CStrProperty, CProperty)
  virtual std::string GetPropertyType() { return "CStrProperty"; }
};

struct CPtrProperty : public CProperty
{
    DECL_CAST(CPtrProperty, CProperty)
    std::shared_ptr<CProperty> PointerToProp;
    virtual std::string GetPropertyType() { return "CPtrProperty"; }
};

struct CObjectPtrProperty : public CPtrProperty {
    DECL_CAST(CObjectPtrProperty, CPtrProperty)
    virtual std::string GetPropertyType() { return "CObjectPtrProperty"; }
};

struct CArrayProperty : public CProperty {
    DECL_CAST(CArrayProperty, CProperty)
    virtual std::string GetPropertyType() { return "CArrayProperty"; }
    std::shared_ptr<CProperty> ElementProp;
};
