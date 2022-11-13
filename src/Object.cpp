#include "Object.h"
#include "RtCXX.h"

RTCXX_NAMESPACE_BEGIN

namespace ObjectInternals
{
	CObjectManager* GObjectManager = GetObjectManager();
	CObjectManager* GetObjectManager()
	{
		static CObjectManager* ObjectManagerPtr =
			[]() ->CObjectManager*{
			static CObjectManager ObjectManager;
			GObjectManager = &ObjectManager;
			return &ObjectManager;
		}();
		return ObjectManagerPtr;
	}

	CObjectManager* GetObjectManager_GlobalVar()
	{
		return GObjectManager;
	}

	void CObjectManager::GarbageCollection()
	{
		std::vector<CGarbageCollector*> RootGarbageCollectors;
		std::vector<CGarbageCollector*> AllGarbageCollectors;
		for (size_t i = 0; i < GarbageCollectorArray.NumChunks; i++)
		{
			CGarbageCollector* GarbageCollectorChunk = GarbageCollectorArray.GetChunk(i);
			for (size_t j = 0; j < GarbageCollectorArray.NumElementsPerChunk; j++)
			{
				if (GarbageCollectorChunk[j].Object)
				{
					AllGarbageCollectors.push_back(&GarbageCollectorChunk[j]);
					if (GarbageCollectorChunk[j].bIsRootCollector)
					{
						RootGarbageCollectors.push_back(&GarbageCollectorChunk[j]);
						GarbageCollectorChunk[j].bIsUnreachable = false;
					}
					else
					{
						GarbageCollectorChunk[j].bIsUnreachable = true;
					}
				}
			}
		}
		

		for (size_t i = 0; i < RootGarbageCollectors.size(); i++)
		{
			auto RootGarbageCollector = RootGarbageCollectors[i];
			CMetaClass* MetaClass = RootGarbageCollector->Object->GetClass();
			CMetaProperty* PropLink = MetaClass->PropertyLink;
			while (PropLink)
			{
				if (PropLink->GetCastFlags() | MCF_CObjectProperty)
				{

				}
				PropLink = PropLink->PropertyLinkNext;
			}
		}
	}

}


//GENERATED_CLASS_IMPL(OObject, void);


RTCXX_NAMESPACE_END
