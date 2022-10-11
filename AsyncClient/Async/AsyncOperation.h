#pragma once

#include <future>

struct AsyncOpVoidReturnType {};

// Concept to ensure classes
template <class TChild, typename TReturnType>
class AsyncOperation
{
public:
	using return_type = TReturnType;

	template <class ... Args>
	static std::future<return_type> ExecuteOperation(Args ... args)
	{
		return std::async(std::launch::async, [=]()
			{
				return TChild::StartOperation(args...);
			});
	}
};

class TestOperation1 : public AsyncOperation<TestOperation1, int>
{
public:

	static return_type StartOperation()
	{
		using namespace std::chrono_literals;

		std::this_thread::sleep_for(2s);

		return 1;
	}
};

class TestOperation2 : public AsyncOperation<TestOperation2, float>
{
public:

	static return_type StartOperation(float increment)
	{
		using namespace std::chrono_literals;

		std::this_thread::sleep_for(2s);

		return 2.f + increment;
	}
};