#include "ReferenceController.h"
#include "../Object.h"

namespace ObjectInternals
{
	TRefControllerManager<ESPMode::ThreadSafe>* GThreadSafeRefControllerManagerPtr = &GetRefControllerManager<ESPMode::ThreadSafe>();
	TRefControllerManager<ESPMode::NotThreadSafe>* GNotThreadSafeRefControllerManagerPtr = &GetRefControllerManager<ESPMode::NotThreadSafe>();

	template<>
	TRefControllerManager<ESPMode::ThreadSafe>& GetRefControllerManager<ESPMode::ThreadSafe>()
	{
		static TRefControllerManager<ESPMode::ThreadSafe>& RefControllerManagerRef =
			[]() ->TRefControllerManager<ESPMode::ThreadSafe>&{
			static TRefControllerManager<ESPMode::ThreadSafe> RefControllerManager;
			GThreadSafeRefControllerManagerPtr = &RefControllerManager;
			return RefControllerManager;
		}();
		return RefControllerManagerRef;
	}

	template<>
	TRefControllerManager<ESPMode::NotThreadSafe>& GetRefControllerManager<ESPMode::NotThreadSafe>()
	{
		static TRefControllerManager<ESPMode::NotThreadSafe>& RefControllerManagerRef =
			[]() ->TRefControllerManager<ESPMode::NotThreadSafe>&{
			static TRefControllerManager<ESPMode::NotThreadSafe> RefControllerManager;
			GNotThreadSafeRefControllerManagerPtr = &RefControllerManager;
			return RefControllerManager;
		}();
		return RefControllerManagerRef;
	}

	template<>
	TRefControllerManager<ESPMode::ThreadSafe>* GetControllerManager_GlobalVar<ESPMode::ThreadSafe>()
	{
		return GThreadSafeRefControllerManagerPtr;
	}
	template<>
	TRefControllerManager<ESPMode::NotThreadSafe>* GetControllerManager_GlobalVar<ESPMode::NotThreadSafe>()
	{
		return GNotThreadSafeRefControllerManagerPtr;
	}

}