#pragma once
#include "ReferenceController.h"

template <class ObjectType, ESPMode Mode = ESPMode::ThreadSafe>
class TWeakObjectPtr
{
	using ElementType = ObjectType;

public:
	FORCEINLINE explicit TWeakObjectPtr()
		: Object(nullptr)
		, WeakReferencer()
	{
	}
	~TWeakObjectPtr() {}

	FORCEINLINE TWeakObjectPtr(TWeakObjectPtr const& InWeakPtr)
		: Object(InWeakPtr.Object)
		, WeakReferencer(InWeakPtr.WeakReferencer)
	{
	}

	FORCEINLINE TWeakObjectPtr(TWeakObjectPtr&& InWeakPtr)
		: Object(InWeakPtr.Object)
		, WeakReferencer(std::move(InWeakPtr.WeakReferencer))
	{
		InWeakPtr.Object = nullptr;
	}

	FORCEINLINE TWeakObjectPtr& operator=(TWeakObjectPtr const& InWeakPtr)
	{
		Object = InWeakPtr.Object;
		WeakReferencer = InWeakPtr.WeakReferencer;
		return *this;
	}

	FORCEINLINE TWeakObjectPtr& operator=(TWeakObjectPtr&& InWeakPtr)
	{
		if (this != &InWeakPtr)
		{
			Object = InWeakPtr.Object;
			InWeakPtr.Object = nullptr;
			WeakReferencer = std::move(InWeakPtr.WeakReferencer);
		}
		return *this;
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr(TSharedObjectPtr<OtherType, Mode> const& InSharedPtr)
		: Object(InSharedPtr.Object)
		, WeakReferencer(InSharedPtr.SharedReferencer)
	{
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr(TWeakObjectPtr<OtherType, Mode> const& InWeakPtr)
		: Object(InWeakPtr.Object)
		, WeakReferencer(InWeakPtr.WeakReferencer)
	{
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr(TWeakObjectPtr<OtherType, Mode>&& InWeakPtr)
		: Object(InWeakPtr.Object)
		, WeakReferencer(MoveTemp(InWeakPtr.WeakReferencer))
	{
		InWeakPtr.Object = nullptr;
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr& operator=(TWeakObjectPtr<OtherType, Mode> const& InWeakPtr)
	{
		Object = InWeakPtr.Object;
		WeakReferencer = InWeakPtr.WeakReferencer;
		return *this;
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr& operator=(TWeakObjectPtr<OtherType, Mode>&& InWeakPtr)
	{
		Object = InWeakPtr.Object;
		InWeakPtr.Object = nullptr;
		WeakReferencer = std::move(InWeakPtr.WeakReferencer);
		return *this;
	}

	template <
		typename OtherType,
		// typename = std::enable_if_t<std::derived_from<OtherType, ObjectType>>
		typename = std::enable_if_t<std::is_base_of_v<ObjectType, OtherType>>>
	FORCEINLINE TWeakObjectPtr& operator=(TSharedObjectPtr<OtherType, Mode> const& InSharedPtr)
	{
		Object = InSharedPtr.Object;
		WeakReferencer = InSharedPtr.SharedReferencer;
		return *this;
	}

	FORCEINLINE TSharedObjectPtr<ObjectType, Mode> TryConvertSharedPtr() const&
	{
		return TSharedObjectPtr<ObjectType, Mode>(*this);
	}

	FORCEINLINE TSharedObjectPtr<ObjectType, Mode> TryConvertSharedPtr() &&
	{
		return TSharedObjectPtr<ObjectType, Mode>(std::move(*this));
	}

	FORCEINLINE const bool IsValid() const
	{
		return Object != nullptr && WeakReferencer.IsValid();
	}

	FORCEINLINE void Reset()
	{
		*this = TWeakObjectPtr<ObjectType, Mode>();
	}

private:
	template <class OtherType, ESPMode OtherMode>
	friend class TSharedObjectPtr;
	template <class OtherType, ESPMode OtherMode>
	friend class TWeakObjectPtr;

public:
	ObjectType* Object;
	ObjectInternals::FWeakReferencer<Mode> WeakReferencer;
};
