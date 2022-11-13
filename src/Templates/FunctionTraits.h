#pragma once

template <typename T>
struct FunctionTraits : FunctionTraits<decltype(&T::operator())>
{
	using ClassType = void;
};

template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...)>
{
	static constexpr size_t SArgCount = sizeof...(Args);
	using ReturnType = Ret;
	using ArgsType = std::tuple<Args...>;
};

template <typename Ret, typename... Args>
struct FunctionTraits<Ret (*)(Args...)> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = void;
};

template <typename Ret, typename... Args>
struct FunctionTraits<Ret (&)(Args...)> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = void;
};

template <typename Ret, typename Cls, typename... Args>
struct FunctionTraits<Ret (Cls::*)(Args...)> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = Cls;
};

template <typename Ret, typename Cls, typename... Args>
struct FunctionTraits<Ret (Cls::*)(Args...) const> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = Cls;
};

template <typename Ret, typename Cls, typename... Args>
struct FunctionTraits<Ret (Cls::*)(Args...) volatile> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = Cls;
};

template <typename Ret, typename Cls, typename... Args>
struct FunctionTraits<Ret (Cls::*)(Args...) const volatile> : public FunctionTraits<Ret(Args...)>
{
	using ClassType = Cls;
};

template <typename T>
struct FunctionTraits<std::function<T>> : FunctionTraits<T>
{
	using ClassType = void;
};
