#include "TestScriptEngine.h"
using namespace RtCXX;

//auto Prop = TProperty<OApplication, decltype(&OApplication::ApplicationName)>::CreateStatic("ApplicationName", &OApplication::ApplicationName, offsetof(OApplication, ApplicationName), OApplication::SelfClass);
//auto Func = TFunction<decltype(&OApplication::MainLoop)>::CreateStatic("MainLoop", &OApplication::MainLoop, OApplication::SelfClass);

//CMetaClass* OApplication::SelfClass = TClass<OApplication>::CreateStatic("OApplication");
TEST_CASE("TestScriptEngine", "[Class]")
{
	//auto Prop = TProperty<OApplication, decltype(&OApplication::ApplicationName)>::CreateStatic("ApplicationName", &OApplication::ApplicationName, offsetof(OApplication, ApplicationName), OApplication::SelfClass);
	//auto Func = TFunction<decltype(&OApplication::MainLoop)>::CreateStatic("MainLoop", &OApplication::MainLoop, OApplication::SelfClass);
	auto Prop = GetControllerPtr()->FindMetadataAs<CMetaProperty>("OApplication::ApplicationName");
	auto Func = GetControllerPtr()->FindMetadataAs<CMetaFunction>("OApplication::MainLoop"); 

	OApplication app;
	OApplication app2;
	app2.ApplicationName = "asd";
	auto ti = GetControllerPtr()->ScriptEngine->GetTypeInfoByDecl("OApplication");
	auto Method = ti->GetMethodByDecl(Func->GetScriptDeclaration().c_str());
	auto Context = GController->ScriptEngine->CreateContext();
	for (size_t i = 0; i < 10; i++)
	{
		int r = Context->Prepare(Method); assert(r >= 0);
		double delta_seconds = 1234.f;
		r = Context->SetArgAddress(0, &app2); assert(r >= 0);
		r = Context->SetArgAddress(1, &delta_seconds); assert(r >= 0);
		r = Context->SetArgObject(1, &delta_seconds); assert(r >= 0);
		//r = Context->SetArgDouble(0, 123); assert(r >= 0);
		r = Context->SetObject(&app); assert(r >= 0);
		r = Context->Execute(); assert(r >= 0);
	}

}

TEST_CASE("TestScriptEngine", "[Function]")
{

}

TEST_CASE("TestScriptEngine", "[Property]")
{
	REQUIRE(CastCheckCastRanges(DerivedABA::SelfClass, BaseA::SelfClass));
	REQUIRE(CastCheckCastRanges(DerivedABA::SelfClass, DerivedAB::SelfClass));

	REQUIRE(CastCheckCastRanges(DerivedAAB::SelfClass, DerivedAA::SelfClass));
	REQUIRE(CastCheckCastRanges(DerivedAAB::SelfClass, BaseA::SelfClass));

	REQUIRE(CastCheckCastRanges(DerivedAAA::SelfClass, BaseA::SelfClass));
	REQUIRE(CastCheckCastRanges(DerivedAAA::SelfClass, DerivedAA::SelfClass));

	REQUIRE(CastCheckCastRanges(DerivedAB::SelfClass, BaseA::SelfClass));
	REQUIRE(CastCheckCastRanges(DerivedAA::SelfClass, BaseA::SelfClass));


	REQUIRE(!CastCheckCastRanges(DerivedABA::SelfClass, DerivedAAB::SelfClass));
	REQUIRE(!CastCheckCastRanges(DerivedAB::SelfClass, DerivedAAA::SelfClass));
	REQUIRE(!CastCheckCastRanges(DerivedAA::SelfClass, DerivedAB::SelfClass));
	REQUIRE(!CastCheckCastRanges(DerivedAA::SelfClass, DerivedABA::SelfClass));
	REQUIRE(!CastCheckCastRanges(DerivedAB::SelfClass, DerivedAAA::SelfClass));
	REQUIRE(!CastCheckCastRanges(DerivedAAA::SelfClass, DerivedAB::SelfClass));
}
