#include "TestScriptEngine.h"
using namespace RtCXX;

//auto Prop = TProperty<OApplication, decltype(&OApplication::ApplicationName)>::CreateStatic("ApplicationName", &OApplication::ApplicationName, offsetof(OApplication, ApplicationName), OApplication::GVar_StaticClass);
//auto Func = TFunction<decltype(&OApplication::MainLoop)>::CreateStatic("MainLoop", &OApplication::MainLoop, OApplication::GVar_StaticClass);

//CMetaClass* OApplication::GVar_StaticClass = TClass<OApplication>::CreateStatic("OApplication");
TEST_CASE("TestScriptEngine", "[Class]")
{
	//auto Prop = TProperty<OApplication, decltype(&OApplication::ApplicationName)>::CreateStatic("ApplicationName", &OApplication::ApplicationName, offsetof(OApplication, ApplicationName), OApplication::GVar_StaticClass);
	//auto Func = TFunction<decltype(&OApplication::MainLoop)>::CreateStatic("MainLoop", &OApplication::MainLoop, OApplication::GVar_StaticClass);
	auto Prop = GetControllerPtr()->FindMetadataAs<CMetaProperty>("OApplication::ApplicationName");
	auto Func = GetControllerPtr()->FindMetadataAs<CMetaFunction>("OApplication::MainLoop"); 

	OApplication app;
	OApplication app2;
	app2.ApplicationName = "asd";
	auto ti = GetControllerPtr()->ScriptEngine->GetTypeInfoByDecl("OApplication");
	auto Method = ti->GetMethodByDecl(Func->GetDeclarationForScriptEngine().c_str());
	auto Context = GController->ScriptEngine->CreateContext();
	for (size_t i = 0; i < 10; i++)
	{
		int r = Context->Prepare(Method); assert(r >= 0);
		double delta_seconds = 1234.f;
		r = Context->SetArgAddress(0, &app2); assert(r >= 0);
		r = Context->SetArgAddress(1, &delta_seconds); assert(r >= 0);
		//r = Context->SetArgObject(1, &delta_seconds); assert(r >= 0);
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
	REQUIRE(CastCheckCastRanges(DerivedABA::GVar_StaticClass, BaseA::GVar_StaticClass));
	REQUIRE(CastCheckCastRanges(DerivedABA::GVar_StaticClass, DerivedAB::GVar_StaticClass));

	REQUIRE(CastCheckCastRanges(DerivedAAB::GVar_StaticClass, DerivedAA::GVar_StaticClass));
	REQUIRE(CastCheckCastRanges(DerivedAAB::GVar_StaticClass, BaseA::GVar_StaticClass));

	REQUIRE(CastCheckCastRanges(DerivedAAA::GVar_StaticClass, BaseA::GVar_StaticClass));
	REQUIRE(CastCheckCastRanges(DerivedAAA::GVar_StaticClass, DerivedAA::GVar_StaticClass));

	REQUIRE(CastCheckCastRanges(DerivedAB::GVar_StaticClass, BaseA::GVar_StaticClass));
	REQUIRE(CastCheckCastRanges(DerivedAA::GVar_StaticClass, BaseA::GVar_StaticClass));


	REQUIRE(!CastCheckCastRanges(DerivedABA::GVar_StaticClass, DerivedAAB::GVar_StaticClass));
	REQUIRE(!CastCheckCastRanges(DerivedAB::GVar_StaticClass, DerivedAAA::GVar_StaticClass));
	REQUIRE(!CastCheckCastRanges(DerivedAA::GVar_StaticClass, DerivedAB::GVar_StaticClass));
	REQUIRE(!CastCheckCastRanges(DerivedAA::GVar_StaticClass, DerivedABA::GVar_StaticClass));
	REQUIRE(!CastCheckCastRanges(DerivedAB::GVar_StaticClass, DerivedAAA::GVar_StaticClass));
	REQUIRE(!CastCheckCastRanges(DerivedAAA::GVar_StaticClass, DerivedAB::GVar_StaticClass));
}
