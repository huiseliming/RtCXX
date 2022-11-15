#pragma once
#include "Templates/SharedObjectPtr.h"

RTCXX_NAMESPACE_BEGIN

namespace ObjectInternals
{

	class CObjectManager;
	RTCXX_API CObjectManager* GetObjectManager();
	RTCXX_API CObjectManager* GetObjectManager_GlobalVar();


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

	struct CGarbageCollector
	{
		I32 Index;
		OObject* Object = nullptr;
		U32 Flag : 24;
		bool bIsRootCollector : 1;
		bool bIsUnreachable : 1;
	};

	class CObjectManager
	{
	public:
		CObjectManager()
		{
			AllocateGarbageCollectorPool();
		}

		void AllocateGarbageCollectorPool(i32 InMaxGarbageCollector = 64 * 1024 * 1024/*, bool PreAllocateGarbageCollectorArray = false*/)
		{
			GarbageCollectorArray.PreAllocate(InMaxGarbageCollector, false);
			for (size_t i = 0; i < GarbageCollectorArray.NumChunks; i++)
			{
				CGarbageCollector* GarbageCollectorChunk = GarbageCollectorArray.GetChunk(i);
				for (size_t j = 0; j < GarbageCollectorArray.NumElementsPerChunk; j++)
				{
					GarbageCollectorChunk[j].Index = i * GarbageCollectorArray.NumElementsPerChunk + j;
				}
			}
		}

		auto AllocateGarbageCollector(OObject* ObjectPtr) -> CGarbageCollector*
		{
			CGarbageCollector* GarbageCollector;
			{
				std::lock_guard<std::mutex> lock(InternalGarbageCollectorsMutex);
				if (!AvailableGarbageCollectorQueue.empty())
				{
					GarbageCollector = AvailableGarbageCollectorQueue.front();
					AvailableGarbageCollectorQueue.pop();
				}
				else
				{
					i32 GarbageCollectorIndex = GarbageCollectorArray.AddSingle();
					GarbageCollector = GarbageCollectorArray.GetElement(GarbageCollectorIndex);
					GarbageCollector->Index = GarbageCollectorIndex;
				}
			}
			assert(GarbageCollector->Object == nullptr);
			GarbageCollector->Object = ObjectPtr;
			return GarbageCollector;
		}

		void FreeGarbageCollector(CGarbageCollector* GarbageCollector)
		{
			assert(GarbageCollector->Object != nullptr);
			GarbageCollector->Object = nullptr;
			std::lock_guard<std::mutex> lock(InternalGarbageCollectorsMutex);
			AvailableGarbageCollectorQueue.push(GarbageCollector);
		}

		void ShutdownGarbageCollectorArray() {}

		void GarbageCollection();

		void ForceGarbageCollection()
		{
		
		}

		std::atomic<bool> bRequestGarbageCollection;
		std::mutex InternalGarbageCollectorsMutex;
		TChunkedArray<CGarbageCollector> GarbageCollectorArray;
		std::queue<CGarbageCollector*> AvailableGarbageCollectorQueue;
	};

}

RTCXX_NAMESPACE_END

class RCLASS() RTCXX_API OObject
{
	GENERATED_BODY();

public:
	OObject()
	{
		printf("OObject::Constructor()");
	}

	virtual ~OObject()
	{
		printf("OObject::Destructor()");
	}

	RtCXX::CMetaClass* GetClass() { return ObjectType; }
	 
protected: 
	void SetObjectType(RtCXX::CMetaClass* type) { ObjectType = type; }
	RFUNCTION()
	void SetObjectType(OObject* ObjectType1) {  }
	 
private:
	RtCXX::CMetaClass* ObjectType;
	RPROPERTY() 
	OObject* ObjectType1; 
	RPROPERTY() 
	OObject* ObjectType3; 
	RPROPERTY() 
	I8* A1;   
	RPROPERTY()
	U64* A2; 
	RPROPERTY() 
	I32* A4; 
	RPROPERTY() 
	F32* A5;
	RPROPERTY()
	std::string *A6;
	RPROPERTY()
	F32 A53;
	RPROPERTY()
	std::string A63;
private:
	template <class ObjectType, ESPMode InMode>
	friend class TSharedObjectPtr;
	template <class ObjectType, ESPMode InMode>
	friend class TWeakObjectPtr;
};

class RCLASS() RTCXX_API OObjectA : public OObject
{
		GENERATED_BODY();

};

