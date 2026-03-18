#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

class DelegateHandle
{
public:
	DelegateHandle()
		: Identifier(0)
	{
	}

	explicit DelegateHandle(std::uint64_t NewIdentifier)
		: Identifier(NewIdentifier)
	{
	}

	bool IsValid() const
	{
		return Identifier != 0;
	}

	void Reset()
	{
		Identifier = 0;
	}

	bool operator==(const DelegateHandle& OtherDelegateHandle) const
	{
		return Identifier == OtherDelegateHandle.Identifier;
	}

	bool operator!=(const DelegateHandle& OtherDelegateHandle) const
	{
		return Identifier != OtherDelegateHandle.Identifier;
	}

private:
	template<typename...>
	friend class MulticastDelegate;

	std::uint64_t Identifier;
};

template<typename... ParameterTypes>
class MulticastDelegate
{
public:
	using DelegateFunction = std::function<void(ParameterTypes...)>;

	MulticastDelegate()
		: NextIdentifier(1)
	{
	}

	DelegateHandle Add(const DelegateFunction& NewDelegateFunction)
	{
		if (!NewDelegateFunction)
		{
			return DelegateHandle();
		}

		DelegateHandle NewDelegateHandle(NextIdentifier);
		NextIdentifier += 1;
		Observers.emplace_back(NewDelegateHandle, NewDelegateFunction);
		return NewDelegateHandle;
	}

	template<typename ObjectType>
	DelegateHandle AddRaw(ObjectType* ObjectInstance, void (ObjectType::*ObjectMethod)(ParameterTypes...))
	{
		if (ObjectInstance == nullptr || ObjectMethod == nullptr)
		{
			return DelegateHandle();
		}

		DelegateFunction NewDelegateFunction =
			[ObjectInstance, ObjectMethod](ParameterTypes... Parameters)
			{
				(ObjectInstance->*ObjectMethod)(Parameters...);
			};
		return Add(NewDelegateFunction);
	}

	template<typename ObjectType>
	DelegateHandle AddRaw(ObjectType* ObjectInstance, void (ObjectType::*ObjectMethod)(ParameterTypes...) const)
	{
		if (ObjectInstance == nullptr || ObjectMethod == nullptr)
		{
			return DelegateHandle();
		}

		DelegateFunction NewDelegateFunction =
			[ObjectInstance, ObjectMethod](ParameterTypes... Parameters)
			{
				(ObjectInstance->*ObjectMethod)(Parameters...);
			};
		return Add(NewDelegateFunction);
	}

	void Remove(DelegateHandle ExistingDelegateHandle)
	{
		if (!ExistingDelegateHandle.IsValid())
		{
			return;
		}

		for (auto ExistingObserverIterator = Observers.begin(); ExistingObserverIterator != Observers.end(); ++ExistingObserverIterator)
		{
			if (ExistingObserverIterator->first == ExistingDelegateHandle)
			{
				Observers.erase(ExistingObserverIterator);
				return;
			}
		}
	}

	void RemoveAll()
	{
		Observers.clear();
	}

	bool IsBound() const
	{
		return !Observers.empty();
	}

	void Broadcast(ParameterTypes... Parameters) const
	{
		const std::vector<std::pair<DelegateHandle, DelegateFunction>> ObserverSnapshot = Observers;
		for (const std::pair<DelegateHandle, DelegateFunction>& ExistingObserver : ObserverSnapshot)
		{
			ExistingObserver.second(Parameters...);
		}
	}

private:
	std::vector<std::pair<DelegateHandle, DelegateFunction>> Observers;
	std::uint64_t NextIdentifier;
};

#define DECLARE_MULTICAST_DELEGATE(DelegateTypeName) \
	using DelegateTypeName = MulticastDelegate<>

#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateTypeName, ParameterType1) \
	using DelegateTypeName = MulticastDelegate<ParameterType1>

#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateTypeName, ParameterType1, ParameterType2) \
	using DelegateTypeName = MulticastDelegate<ParameterType1, ParameterType2>

#define DECLARE_MULTICAST_DELEGATE_ThreeParams(DelegateTypeName, ParameterType1, ParameterType2, ParameterType3) \
	using DelegateTypeName = MulticastDelegate<ParameterType1, ParameterType2, ParameterType3>

#define DECLARE_MULTICAST_DELEGATE_FourParams(DelegateTypeName, ParameterType1, ParameterType2, ParameterType3, ParameterType4) \
	using DelegateTypeName = MulticastDelegate<ParameterType1, ParameterType2, ParameterType3, ParameterType4>
