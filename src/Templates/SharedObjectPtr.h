#pragma once
#include "ReferenceController.h"
#include "WeakObjectPtr.h"

// https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync

template <class ObjectType, ESPMode Mode = ESPMode::ThreadSafe>
class TSharedObjectPtr
{

public:
	FORCEINLINE explicit TSharedObjectPtr()
		: Object(nullptr)
		, SharedReferencer()
	{
	}

	~TSharedObjectPtr() {}

	FORCEINLINE explicit TSharedObjectPtr(OObject* InObject)
		: Object(InObject)
		, SharedReferencer(InObject ? ObjectInternals::GetControllerManager_GlobalVar<Mode>()->AllocateRefController(InObject) : nullptr)
	{
	}

	FORCEINLINE TSharedObjectPtr(TSharedObjectPtr const& InSharedPtr)
		: Object(InSharedPtr.Object)
		, SharedReferencer(InSharedPtr.SharedReferencer)
	{
	}

	FORCEINLINE TSharedObjectPtr(TSharedObjectPtr&& InSharedPtr)
		: Object(InSharedPtr.Object)
		, SharedReferencer(std::move(InSharedPtr.SharedReferencer))
	{
		InSharedPtr.Object = nullptr;
	}

	FORCEINLINE TSharedObjectPtr& operator=(TSharedObjectPtr const& InSharedPtr)
	{
		Object = InSharedPtr.Object;
		SharedReferencer = InSharedPtr.SharedReferencer;
		return *this;
	}

	FORCEINLINE TSharedObjectPtr& operator=(TSharedObjectPtr&& InSharedPtr)
	{
		if (this != &InSharedPtr)
		{
			Object = InSharedPtr.Object;
			InSharedPtr.Object = nullptr;
			SharedReferencer = std::move(InSharedPtr.SharedReferencer);
		}
		return *this;
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TSharedObjectPtr(TSharedObjectPtr<OtherType, Mode> const& InSharedPtr)
		: Object(InSharedPtr.Object)
		, SharedReferencer(InSharedPtr.SharedReferencer)
	{
	}

	FORCEINLINE ObjectType* Get() const
	{
		return Object;
	}

	FORCEINLINE explicit operator bool() const
	{
		return Object != nullptr;
	}

	FORCEINLINE const bool IsValid() const
	{
		return Object != nullptr;
	}

	FORCEINLINE ObjectType* operator->() const
	{
		return Object;
	}

	FORCEINLINE void Reset()
	{
		*this = TSharedObjectPtr<ObjectType, Mode>();
	}
	FORCEINLINE const i32 GetSharedReferenceCount() const
	{
		return SharedReferencer.GetSharedReferenceCount();
	}

private:
	// From WeakObjectPtr
	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TSharedObjectPtr(TWeakObjectPtr<OtherType, Mode> const& InWeakPtr)
		: Object(nullptr)
		, SharedReferencer(InWeakPtr.WeakReferencer)
	{
		if (SharedReferencer.IsValid())
		{
			Object = InWeakPtr.Object;
		}
	}
	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TSharedObjectPtr(TWeakObjectPtr<OtherType, Mode>&& InWeakPtr)
		: Object(nullptr)
		, SharedReferencer(std::move(InWeakPtr.WeakReferencer))
	{
		if (SharedReferencer.IsValid())
		{
			Object = InWeakPtr.Object;
			InWeakPtr.Object = nullptr;
		}
	}

	template <class OtherType, ESPMode OtherMode>
	friend class TSharedObjectPtr;
	template <class OtherType, ESPMode OtherMode>
	friend class TWeakObjectPtr;

private:
	ObjectType* Object;
	ObjectInternals::FSharedReferencer<Mode> SharedReferencer;
};
