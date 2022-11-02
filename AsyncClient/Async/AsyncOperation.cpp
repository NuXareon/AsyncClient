#include "AsyncOperation.h"

#include "../Utils.h"

#include <iostream>

TestOperation1::async_return_type TestOperation1::StartOperation()
{
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(2s);

	return 1;
}

TestOperation2::async_return_type TestOperation2::StartOperation(float increment)
{
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(2s);

	return 2.f + increment;
}