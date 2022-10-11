// AsyncClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <future>
#include <coroutine>

#include "Async/AsyncOperation.h"
#include "Async/Coroutine.h"

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
    int result3 = co_await StartAsyncCoroutineOperation<TestOperation1>();

    co_return result2+result3;
}

// Parallel coroutine with return type
AsyncTask<float> testCoroutineParallel()
{
    // TODO: make a single coroutine for this?
    auto operation1 = StartAsyncCoroutineOperation<TestOperation2>(1.0f);
    auto operation2 = StartAsyncCoroutineOperation<TestOperation2>(1.0f);

    // Doing the loop this ways avoids waiting and extra frame at the end of the operations
    while (true)
    {
        operation1();
        operation2();
        if (operation1.IsFinished() && operation2.IsFinished())
            break; // Could do co_return here too

        co_await std::suspend_always{};
    } 

    co_return operation1.GetResult() + operation2.GetResult();
}

// Parallel coroutine usign weird templated functions
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
    /*
    // Both start here, we need to tick them to know when they finish
    auto operation1 = StartAsyncCoroutineOperation<TestOperation1>();
    auto operation2 = StartAsyncCoroutineOperation<TestOperation2>(0.5f);

    // Wait for operations to finish
    while (!operation1.IsFinished())
    {
        operation1();
    }
    std::cout << operation1.GetResult() << "\n";

    while (!operation2.IsFinished())
    {
        operation2();
    }
    std::cout << operation2.GetResult() << "\n";

    
    ExecuteAsyncCoroutineOperation<TestOperation2>(2.0f);

    auto testOperation1Task = CreateAsyncCoroutineOperation<TestOperation1>();

    while (!testOperation1Task.IsFinished())
    {
        testOperation1Task();
    }
    const auto resultCoOp1 = testOperation1Task.GetResult();
    std::cout << resultCoOp1 << "\n";

    AsyncTask c = testCoroutine();
    while (!c.IsFinished())
    {
        c();
    }
    const float resultCo = c.GetResult();
    std::cout << resultCo << "\n";

    auto asyncResult1 = TestOperation1::ExecuteOperation();

    while (asyncResult1.wait_for(0s) != std::future_status::ready)
    {
        // wait
    }

    const auto result1 = asyncResult1.get();
    std::cout << result1 << "\n";

    auto asyncResult2 = TestOperation2::ExecuteOperation(0.5f);

    while (asyncResult2.wait_for(0s) != std::future_status::ready)
    {
        // wait
    }

    const auto result2 = asyncResult2.get();
    std::cout << result2 << "\n";
    */
}