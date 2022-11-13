#pragma once
#include "MetaClass.h"
#include "Object.h"

RTCXX_NAMESPACE_BEGIN

FORCEINLINE bool CastRecursively(CMetaClass* From, CMetaClass* To)
{
	auto CurrentTarget = To->ExtendsClass;
	while (CurrentTarget)
	{
		if (CurrentTarget == From)
			return true;
		CurrentTarget = CurrentTarget->ExtendsClass;
	}
	return false;
}

FORCEINLINE bool CastCheckCastRanges(CMetaClass* From, CMetaClass* To)
{
	if (To->CastRanges.first <= From->CastRanges.first &&
		From->CastRanges.first < To->CastRanges.second)
		return true;
	return false;
}

template<typename T>
OObject* CastTo(OObject* Object)
{
	if (CastCheckCastRanges(Object->GetClass(), T::SelfClass))
	{
		return static_cast<T*>(Object);
	}
	return nullptr;
}
RTCXX_NAMESPACE_END