// AsyncClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Async/AsyncCoroutineOperation.h"

using namespace std::chrono_literals;

// Coroutine with no return type
AsyncTask<> testCoroutine1()
{
    std::cout << "Im a coroutine \n";
    co_await std::suspend_always{};
    std::cout << "Im a coroutine again \n";
    co_return;
}

// Sequential coroutine with return type
AsyncTask<float> testCoroutine2()
{
    co_await testCoroutine1(); // no return type
    float result1 = co_await StartAsyncCoroutineOperation<TestOperation2>(1.0f);
    result1 += 1.0f;
    float result2 = co_await StartAsyncCoroutineOperation<TestOperation2>(result1);
    int result3 = co_await StartAsyncCoroutineOperation<TestOperation1>();  // This could be done in parallel since it doesn't depend on the previous ops.

    co_return result2+result3;
}

// Parallel coroutine with return type
AsyncTask<float> testCoroutineParallel()
{
    auto operation1 = StartAsyncCoroutineOperation<TestOperation2>(1.0f);
    auto operation2 = StartAsyncCoroutineOperation<TestOperation2>(1.0f);

    // Doing the loop this ways avoids waiting and extra frame at the end of the operations
    while (true)
    {
        operation1();
        operation2();
        if (operation1.IsFinished() && operation2.IsFinished())
            break; // Could do co_return here

        co_await std::suspend_always{};
    } 

    co_return operation1.GetResult() + operation2.GetResult();
}

// Parallel coroutines usign templated functions (No return)
AsyncTask<> testCoroutineParallel2()
{
    auto operation1 = StartAsyncCoroutineOperation<TestOperation1>();
    auto operation2 = StartAsyncCoroutineOperation<TestOperation2>(1.0f);
    auto operation3 = StartAsyncCoroutineOperation<TestOperation1>();
    auto operation4 = StartAsyncCoroutineOperation<TestOperation1>();
    auto operation5 = StartAsyncCoroutineOperation<TestOperation2>(2.0f);
    auto operation6 = StartAsyncCoroutineOperation<TestOperation2>(6.0f);

    co_await ExecuteParallelOperations(operation1, operation2);

    co_await ExecuteParallelOperations(operation3, operation4, operation5, operation6);

    co_return;
}

int main()
{
    // TODO: coroutine manager to run all this tasks in an easy way
    auto operationtest = testCoroutine2();
    
    while (!operationtest.IsFinished())
    {
        operationtest();
    }
    std::cout << "First taks done: " << operationtest.GetResult() << ", expected: 7\n";

    auto operationtestparallel = testCoroutineParallel();

    while (!operationtestparallel.IsFinished())
    {
        operationtestparallel();
    }
    std::cout << "Parallel task done:" << operationtestparallel.GetResult() << ", expected: 6\n";

    auto operationtestparallelfunct = testCoroutineParallel2();

    while (!operationtestparallelfunct.IsFinished())
    {
        operationtestparallelfunct();
    }
    std::cout << "Parallel task function done\n";

    return 0;
}

