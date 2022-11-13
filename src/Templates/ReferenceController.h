#pragma once
#include "../Fwd.h"
#include <boost/detail/interlocked.hpp>
#include <mutex>
#include <queue>

enum class ESPMode : u8
{
	NotThreadSafe = 0,
	ThreadSafe = 1,
};

template <class OtherType, ESPMode OtherMode>
class TSharedObjectPtr;
template <class OtherType, ESPMode OtherMode>
class TWeakObjectPtr;

namespace ObjectInternals
{
	template<ESPMode Mode>
	class TRefControllerManager;

	template<ESPMode Mode>
	TRefControllerManager<Mode>& GetRefControllerManager();
	template<>
	RTCXX_API TRefControllerManager<ESPMode::ThreadSafe>& GetRefControllerManager<ESPMode::ThreadSafe>();
	template<>
	RTCXX_API TRefControllerManager<ESPMode::NotThreadSafe>& GetRefControllerManager<ESPMode::NotThreadSafe>();

	template<ESPMode Mode>
	TRefControllerManager<Mode>* GetControllerManager_GlobalVar();
	template<>
	RTCXX_API TRefControllerManager<ESPMode::ThreadSafe>* GetControllerManager_GlobalVar<ESPMode::ThreadSafe>();
	template<>
	RTCXX_API TRefControllerManager<ESPMode::NotThreadSafe>* GetControllerManager_GlobalVar<ESPMode::NotThreadSafe>();

	template<ESPMode Mode> class FWeakReferencer;

	template<ESPMode Mode>
	class TReferenceController
	{
		using RefCountType = std::conditional_t<Mode == ESPMode::ThreadSafe, std::atomic<i32>, i32>;
	public:
		FORCEINLINE explicit TReferenceController() = default;
		virtual ~TReferenceController() {}
		void DestroyObject() { delete Object; };

		FORCEINLINE i32 GetSharedReferenceCount() const
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				//仅需要保证原子性
				return SharedReferencer.load(std::memory_order_relaxed);
			}
			else
			{
				return SharedReferencer;
			}
		}

		FORCEINLINE void AddSharedReference()
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				SharedReferencer.fetch_add(1, std::memory_order_relaxed);
			}
			else
			{
				++SharedReferencer;
			}
		}

		bool ConditionallyAddSharedReference()
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				i32 OriginalCount = SharedReferencer.load(std::memory_order_relaxed);
				for (; ; )
				{
					if (OriginalCount == 0)
					{
						return false;
					}
					if (SharedReferencer.compare_exchange_weak(OriginalCount, OriginalCount + 1, std::memory_order_relaxed))
					{
						return true;
					}
				}
			}
			else
			{
				if (SharedReferencer == 0)
				{
					return false;
				}
				++SharedReferencer;
				return true;
			}
		}

		FORCEINLINE void ReleaseSharedReference()
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				i32 OldSharedCount = SharedReferencer.fetch_sub(1, std::memory_order_release);
				if (OldSharedCount == 1)
				{
					// 保证其他线程的所有的ReleaseSharedReference()调用之前的任何操作在此线程可见
					std::atomic_thread_fence(std::memory_order_acquire);
					DestroyObject();
					ReleaseWeakReference();
				}
			}
			else
			{
				if (--SharedReferencer == 0)
				{
					DestroyObject();
					ReleaseWeakReference();
				}
			}
		}

		FORCEINLINE void AddWeakReference()
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				WeakReferencer.fetch_add(1, std::memory_order_relaxed);
			}
			else
			{
				++WeakReferencer;
			}
		}

		FORCEINLINE void ReleaseWeakReference()
		{
			if constexpr (Mode == ESPMode::ThreadSafe)
			{
				i32 OldWeakCount = WeakReferencer.fetch_sub(1, std::memory_order_release);
				if (OldWeakCount == 1)
				{
					// 保证其他线程的所有的ReleaseWeakReference()调用之前的任何操作在此线程可见
					std::atomic_thread_fence(std::memory_order_acquire);
					GetControllerManager_GlobalVar<Mode>()->FreeRefController(this);
				}
			}
			else
			{
				if (--WeakReferencer == 0)
				{
					GetControllerManager_GlobalVar<Mode>()->FreeRefController(this);
				}
			}
		}

		OObject* Object = nullptr;
		i32 ArrayIndex = -1;
		RefCountType SharedReferencer = 1;
		RefCountType WeakReferencer = 1;

	private:
		// Non-copyable
		TReferenceController(const TReferenceController&) = delete;
		TReferenceController& operator=(const TReferenceController&) = delete;
	};


	template< ESPMode Mode >
	class FSharedReferencer
	{
	public:
		FORCEINLINE FSharedReferencer()
			: ReferenceController(nullptr)
		{ }

		inline explicit FSharedReferencer(TReferenceController<Mode>* InReferenceController)
			: ReferenceController(InReferenceController)
		{ }

		FORCEINLINE FSharedReferencer(FSharedReferencer const& InSharedReferencer)
			: ReferenceController(InSharedReferencer.ReferenceController)
		{
			if (ReferenceController != nullptr)
			{
				ReferenceController->AddSharedReference();
			}
		}

		FORCEINLINE FSharedReferencer(FSharedReferencer&& InSharedReferencer)
			: ReferenceController(InSharedReferencer.ReferenceController)
		{
			InSharedReferencer.ReferenceController = nullptr;
		}

		FSharedReferencer(FWeakReferencer< Mode > const& InWeakReferencer)
			: ReferenceController(InWeakReferencer.ReferenceController)
		{
			if (ReferenceController != nullptr)
			{
				if (!ReferenceController->ConditionallyAddSharedReference())
				{
					ReferenceController = nullptr;
				}
			}
		}

		FSharedReferencer(FWeakReferencer< Mode >&& InWeakReferencer)
			: ReferenceController(InWeakReferencer.ReferenceController)
		{
			if (ReferenceController != nullptr)
			{
				if (!ReferenceController->ConditionallyAddSharedReference())
				{
					ReferenceController = nullptr;
				}

				InWeakReferencer.ReferenceController->ReleaseWeakReference();
				InWeakReferencer.ReferenceController = nullptr;
			}
		}

		/** Destructor. */
		FORCEINLINE ~FSharedReferencer()
		{
			if (ReferenceController != nullptr)
			{
				ReferenceController->ReleaseSharedReference();
			}
		}

		inline FSharedReferencer& operator=(FSharedReferencer const& InSharedReferencer)
		{
			auto NewReferenceController = InSharedReferencer.ReferenceController;
			if (NewReferenceController != ReferenceController)
			{
				if (NewReferenceController != nullptr)
				{
					NewReferenceController->AddSharedReference();
				}
				if (ReferenceController != nullptr)
				{
					ReferenceController->ReleaseSharedReference();
				}
				ReferenceController = NewReferenceController;
			}

			return *this;
		}

		inline FSharedReferencer& operator=(FSharedReferencer&& InSharedReferencer)
		{
			auto NewReferenceController = InSharedReferencer.ReferenceController;
			auto OldReferenceController = ReferenceController;
			if (NewReferenceController != OldReferenceController)
			{
				InSharedReferencer.ReferenceController = nullptr;
				ReferenceController = NewReferenceController;

				if (OldReferenceController != nullptr)
				{
					OldReferenceController->ReleaseSharedReference();
				}
			}

			return *this;
		}

		FORCEINLINE const bool IsValid() const
		{
			return ReferenceController != nullptr;
		}

		FORCEINLINE const i32 GetSharedReferenceCount() const
		{
			return ReferenceController != nullptr ? ReferenceController->GetSharedReferenceCount() : 0;
		}

		FORCEINLINE const bool IsUnique() const
		{
			return GetSharedReferenceCount() == 1;
		}

	private:
		template< ESPMode OtherMode > friend class FWeakReferencer;
	private:
		TReferenceController<Mode>* ReferenceController;
	};

	template< ESPMode Mode >
	class FWeakReferencer
	{
	public:
		FORCEINLINE FWeakReferencer()
			: ReferenceController(nullptr)
		{ }

		FORCEINLINE FWeakReferencer(FWeakReferencer const& InWeakReferencer)
			: ReferenceController(InWeakReferencer.ReferenceController)
		{
			if (ReferenceController != nullptr)
			{
				ReferenceController->AddWeakReference();
			}
		}

		FORCEINLINE FWeakReferencer(FWeakReferencer&& InWeakReferencer)
			: ReferenceController(InWeakReferencer.ReferenceController)
		{
			InWeakReferencer.ReferenceController = nullptr;
		}

		FORCEINLINE FWeakReferencer(FSharedReferencer< Mode > const& InSharedReferencer)
			: ReferenceController(InSharedReferencer.ReferenceController)
		{
			if (ReferenceController != nullptr)
			{
				ReferenceController->AddWeakReference();
			}
		}

		FORCEINLINE ~FWeakReferencer()
		{
			if (ReferenceController != nullptr)
			{
				ReferenceController->ReleaseWeakReference();
			}
		}

		FORCEINLINE FWeakReferencer& operator=(FWeakReferencer const& InWeakReferencer)
		{
			AssignReferenceController(InWeakReferencer.ReferenceController);
			return *this;
		}

		FORCEINLINE FWeakReferencer& operator=(FWeakReferencer&& InWeakReferencer)
		{
			auto OldReferenceController = ReferenceController;
			ReferenceController = InWeakReferencer.ReferenceController;
			InWeakReferencer.ReferenceController = nullptr;
			if (OldReferenceController != nullptr)
			{
				OldReferenceController->ReleaseWeakReference();
			}
			return *this;
		}

		FORCEINLINE FWeakReferencer& operator=(FSharedReferencer< Mode > const& InSharedReferencer)
		{
			AssignReferenceController(InSharedReferencer.ReferenceController);
			return *this;
		}

		FORCEINLINE const bool IsValid() const
		{
			return ReferenceController != nullptr && ReferenceController->GetSharedReferenceCount() > 0;
		}

	private:
		inline void AssignReferenceController(TReferenceController<Mode>* NewReferenceController)
		{
			if (NewReferenceController != ReferenceController)
			{
				if (NewReferenceController != nullptr)
				{
					NewReferenceController->AddWeakReference();
				}
				if (ReferenceController != nullptr)
				{
					ReferenceController->ReleaseWeakReference();
				}
				ReferenceController = NewReferenceController;
			}
		}

	private:
		template< ESPMode OtherMode > friend class FSharedReferencer;
	private:
		TReferenceController<Mode>* ReferenceController;
	};

	template<typename ElementType>
	struct TChunkedArray
	{
		static constexpr i32 NumElementsPerChunk = 64 * 1024;
		ElementType** Elements;
		//ElementType* PreAllocatedElements;
		i32 MaxElements;
		i32 NumElements;
		i32 MaxChunks;
		i32 NumChunks;

		void ExpandChunksToIndex(i32 index)
		{
			assert(index >= 0 && index < MaxElements);
			i32 ChunkIndex = index / NumElementsPerChunk;
			ElementType* NewChunk = nullptr;
			while (ChunkIndex >= NumChunks)
			{
				ElementType** chunk = &Elements[NumChunks];
				if (NewChunk == nullptr) NewChunk = new ElementType[NumElementsPerChunk];
				if (BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER((void**)chunk, NewChunk, nullptr))
				{

				}
				else
				{
					NewChunk = nullptr;
					NumChunks++;
					assert(NumChunks <= MaxChunks);
				}
			}
			if (NewChunk) delete[] NewChunk;
			NewChunk = nullptr;
			assert(ChunkIndex < NumChunks&& Elements[ChunkIndex]); // should have a valid pointer now
		}
	public:
		TChunkedArray()
			: Elements(nullptr)
			//, PreAllocatedElements(nullptr)
			, MaxElements(0)
			, NumElements(0)
			, MaxChunks(0)
			, NumChunks(0)
		{}

		~TChunkedArray()
		{
			for (i32 ChunkIndex = 0; ChunkIndex < NumChunks; ++ChunkIndex)
			{
				delete[] Elements[ChunkIndex];
			}
			delete[] Elements;
		}

		/**
		* Expands the array so that Element[index] is allocated. New pointers are all zero.
		* @param index The index of an element we want to be sure is allocated
		**/
		void PreAllocate(i32 InMaxElements, bool InPreAllocateChunks)
		{
			assert(!Elements);
			MaxChunks = (InMaxElements - 1) / NumElementsPerChunk + 1;
			MaxElements = MaxChunks * NumElementsPerChunk;
			Elements = new ElementType * [MaxChunks];
			std::memset(Elements, 0, sizeof(ElementType*) * MaxChunks);
			if (InPreAllocateChunks)
			{
				for (i32 ChunkIndex = 0; ChunkIndex < MaxChunks; ++ChunkIndex)
				{
					Elements[ChunkIndex] = new ElementType[NumElementsPerChunk];
				}
				NumChunks = MaxChunks;
			}
		}

		FORCEINLINE i32 Num() const
		{
			return NumElements;
		}

		FORCEINLINE i32 Capacity() const
		{
			return MaxElements;
		}

		FORCEINLINE bool IsValidIndex(i32 Index) const
		{
			return Index < Num() && Index >= 0;
		}

		FORCEINLINE ElementType const* GetElement(i32 Index) const
		{
			const i32 ChunkIndex = Index / NumElementsPerChunk;
			const i32 WithinChunkIndex = Index % NumElementsPerChunk;
			ASSERT_FORMAT(IsValidIndex(Index), "IsValidIndex(%d)", Index);
			ASSERT_FORMAT(ChunkIndex < NumChunks, "ChunkIndex (%d) < NumChunks (%d)", ChunkIndex, NumChunks);
			ASSERT_FORMAT(Index < MaxElements, "Index (%d) < MaxElements (%d)", Index, MaxElements);
			ElementType* chunk = Elements[ChunkIndex];
			ASSERT_FORMAT(chunk);
			return chunk + WithinChunkIndex;
		}

		FORCEINLINE ElementType* GetElement(i32 Index)
		{
			const i32 ChunkIndex = Index / NumElementsPerChunk;
			const i32 WithinChunkIndex = Index % NumElementsPerChunk;
			ASSERT_FORMAT(IsValidIndex(Index), "IsValidIndex(%d)", Index);
			ASSERT_FORMAT(ChunkIndex < NumChunks, "ChunkIndex (%d) < NumChunks (%d)", ChunkIndex, NumChunks);
			ASSERT_FORMAT(Index < MaxElements, "Index (%d) < MaxElements (%d)", Index, MaxElements);
			ElementType* chunk = Elements[ChunkIndex];
			ASSERT_FORMAT(chunk);
			return chunk + WithinChunkIndex;
		}

		FORCEINLINE ElementType const* GetChunk(i32 ChunkIndex) const
		{
			ElementType* chunk = Elements[ChunkIndex];
			return chunk;
		}

		FORCEINLINE ElementType* GetChunk(i32 ChunkIndex)
		{
			ElementType* chunk = Elements[ChunkIndex];
			return chunk;
		}

		FORCEINLINE ElementType const& operator[](i32 Index) const
		{
			ElementType const* ElementPtr = GetElement(Index);
			return *ElementPtr;
		}

		FORCEINLINE ElementType& operator[](i32 Index)
		{
			ElementType* ElementPtr = GetElement(Index);
			return *ElementPtr;
		}

		i32 AddRange(i32 NumToAdd)
		{
			i32 result = NumElements;
			ASSERT_FORMAT(result + NumToAdd <= MaxElements, "Maximum number of UObjects (%d) exceeded, make sure you update MaxObjectsInGame/MaxObjectsInEditor/MaxObjectsInProgram in project settings.", MaxElements);
			ExpandChunksToIndex(result + NumToAdd - 1);
			NumElements += NumToAdd;
			return result;
		}

		i32 AddSingle()
		{
			return AddRange(1);
		}

		//i32 GetAllocatedSize() const
		//{
		//	return MaxChunks * sizeof(ElementType*) + NumChunks * NumElementsPerChunk * sizeof(ElementType);
		//}

	};

	template<ESPMode Mode>
	class TRefControllerManager
	{
	public:
		TRefControllerManager()
		{
			AllocateRefControllerPool();
		}

		void AllocateRefControllerPool(i32 InMaxRefController = 64 * 1024 * 1024, bool PreAllocateRefControllerArray = false)
		{
			GarbageCollectorArray.PreAllocate(InMaxRefController, PreAllocateRefControllerArray);
			for (size_t i = 0; i < GarbageCollectorArray.NumChunks; i++)
			{
				TReferenceController<Mode>* RefControllerChunk = GarbageCollectorArray.GetChunk(i);
				for (size_t j = 0; j < GarbageCollectorArray.NumElementsPerChunk; j++)
				{
					RefControllerChunk[j].ArrayIndex = i * GarbageCollectorArray.NumElementsPerChunk + j;
				}
			}
		}

		auto AllocateRefController(OObject* ObjectPtr) -> TReferenceController<Mode>*
		{
			TReferenceController<Mode>* RefController;
			{
				std::lock_guard<std::mutex> lock(InternalRefControllersMutex);
				if (!AvailableRefControllerQueue.empty())
				{
					RefController = AvailableRefControllerQueue.front();
					AvailableRefControllerQueue.pop();
				}
				else
				{
					i32 RefControllerIndex = GarbageCollectorArray.AddSingle();
					RefController = GarbageCollectorArray.GetElement(RefControllerIndex);
					RefController->ArrayIndex = RefControllerIndex;
				}
			}
			RefController->Object = ObjectPtr;
			return RefController;
		}

		void FreeRefController(TReferenceController<Mode>* RefController)
		{
			RefController->Object = nullptr;
			std::lock_guard<std::mutex> lock(InternalRefControllersMutex);
			AvailableRefControllerQueue.push(RefController);
		}

		void ShutdownRefControllerArray() {}

		std::mutex InternalRefControllersMutex;
		TChunkedArray<TReferenceController<Mode>> GarbageCollectorArray;
		std::queue<TReferenceController<Mode>*> AvailableRefControllerQueue;
	};

}


