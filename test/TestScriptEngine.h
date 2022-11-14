#pragma once
#include <catch2/catch_all.hpp>
#include "RtCXX.h"
#include "Cast.h" 

class RCLASS() BaseA
{
	GENERATED_BODY()
public:
	RFUNCTION()
	int FuncA(int i1, int i2, int i3) { 
		return i1 + i2 + i3; 
	}
	i8 PropA; 
};

struct RCLASS() DerivedAA : public BaseA
{
	GENERATED_BODY()
		RFUNCTION()
		int FuncAA(int i1, int i2, int i3) {
		return i1 + i2 + i3;
	}	
	
	RPROPERTY()
	i8 PropAA;
};

struct RCLASS() DerivedAB : public BaseA
{
	GENERATED_BODY()
		RFUNCTION()
		int FuncAB(int i1, int i2, int i3) {
		return i1 + i2 + i3;
	}
	RPROPERTY()
	i8 PropAB;
};

struct RCLASS() DerivedAAA : public DerivedAA
{
	GENERATED_BODY()

	RFUNCTION()
	int FuncAAA(int i1, int i2, int i3) {
		return i1 + i2 + i3;
	}
	RPROPERTY()
	i8 PropAAA;
};

struct RCLASS() DerivedAAB : public DerivedAA
{
	GENERATED_BODY()
		RFUNCTION()
		int FuncAAB(int i1, int i2, int i3) {
		return i1 + i2 + i3;
	}
	RPROPERTY()
	i8 PropAAB;
};

struct RCLASS() DerivedABA : public DerivedAB
{
	GENERATED_BODY()
		RFUNCTION()
		int FuncABA(int i1, int i2, int i3) {
		return i1 + i2 + i3;
	}
	RPROPERTY()
	i8 PropABA;
};

class RCLASS() OApplication
{
	GENERATED_BODY()
public:
	RFUNCTION()
	void StartUp()
	{

	}
	RFUNCTION()
	void MainLoop(OApplication* app, double& delta_seconds)
	{
		printf("%f", delta_seconds);
	}
	RFUNCTION()
	void CleanUp()
	{
	 
	} 
	RFUNCTION()
	void RequestExit()
	{
		bRequestExit = true;
	}
	RFUNCTION()
	bool ShouldExit()
	{
		return bRequestExit;
	}

	// void Terminate()
	RPROPERTY()
	bool bRequestExit = false;

	RPROPERTY()
	std::string ApplicationName;
};
