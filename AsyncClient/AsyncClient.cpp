// AsyncClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <grpcpp/create_channel.h>

#include "Async/AsyncCoroutineOperation.h"
#include "Async/BookStatusService.h"
#include "UI/DisplayBooksByYearUI.h"
#include "UI/ManageBookReservationsUI.h"

using namespace std::chrono_literals;

class TestOperation1 : public Async::Operation<TestOperation1, int>
{
public:

    static async_return_type StartOperation()
    {
        std::this_thread::sleep_for(2s);
    
        return 1;
    }
};

class TestOperation2 : public Async::Operation<TestOperation2, float>
{
public:

    static async_return_type StartOperation(float increment)
    {
        std::this_thread::sleep_for(2s);

        return 2.f + increment;
    }
};

// Coroutine with no return type
Async::Task<> testCoroutine1()
{
    std::cout << "Im a coroutine \n";
    co_await std::suspend_always{};
    std::cout << "Im a coroutine again \n";
    co_return;
}

// Sequential coroutine with return type
Async::Task<float> testCoroutine2()
{
    co_await testCoroutine1(); // no return type
    auto result1 = co_await Async::RunOperation<TestOperation2>(1.0f);
    result1.mReturnValue += 1.0f;
    auto result2 = co_await Async::RunOperation<TestOperation2>(result1.mReturnValue);
    auto result3 = co_await Async::RunOperation<TestOperation1>();  // This could be done in parallel since it doesn't depend on the previous ops.

    co_return result2.mReturnValue + result3.mReturnValue;
}

// Parallel coroutine with return type
Async::Task<float> testCoroutineParallel()
{
    auto operation1 = Async::RunOperation<TestOperation2>(1.0f);
    auto operation2 = Async::RunOperation<TestOperation2>(1.0f);

    // Doing the loop this ways avoids waiting and extra frame at the end of the operations
    while (true)
    {
        operation1();
        operation2();
        if (operation1.IsFinished() && operation2.IsFinished())
            break; // Could do co_return here

        co_await std::suspend_always{};
    } 

    co_return operation1.GetResult().mReturnValue + operation2.GetResult().mReturnValue;
}

// Parallel coroutines usign templated functions (No return)
Async::Task<> testCoroutineParallel2()
{
    auto operation1 = Async::RunOperation<TestOperation1>();
    auto operation2 = Async::RunOperation<TestOperation2>(1.0f);
    auto operation3 = Async::RunOperation<TestOperation1>();
    auto operation4 = Async::RunOperation<TestOperation1>();
    auto operation5 = Async::RunOperation<TestOperation2>(2.0f);
    auto operation6 = Async::RunOperation<TestOperation2>(6.0f);

    co_await Async::RunOperationsParallel(operation1, operation2);

    co_await Async::RunOperationsParallel(operation3, operation4, operation5, operation6);

    co_return;
}

void ExecuteCoroutineTest()
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
}

void ExecuteBookDataExample()
{
    DisplayBooksByYearUI bookUI;
    long long frameCount = 0;
    bool runSimulation = true;

    bookUI.Start();

    while (runSimulation)
    {
        runSimulation &= bookUI.Tick(frameCount);

        // ~60fps (minus UI tick)
        constexpr auto sleepTime = std::chrono::milliseconds(static_cast<int>(1000.0f / 60.0f));
        std::this_thread::sleep_for(sleepTime);
        ++frameCount;
    }

    bookUI.End();
}

void ExecuteBookStatusExample()
{
    auto bookStatusService = std::make_shared<BookStatusService>(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    ManageBookReservationsUI bookUI(bookStatusService);

    long long frameCount = 0;
    bool runSimulation = true;

    bookUI.Start();

    while (runSimulation)
    {
        runSimulation &= bookUI.Tick(frameCount);

        // ~60fps (minus UI tick)
        constexpr auto sleepTime = std::chrono::milliseconds(static_cast<int>(1000.0f / 60.0f));
        std::this_thread::sleep_for(sleepTime);
        ++frameCount;
    }

    bookUI.End();
}

int main()
{
    while (1)
    {
        std::cout 
            << "1 - Run Coroutine Test" << std::endl 
            << "2 - Book Info UI" << std::endl
            << "3 - Book Status UI" << std::endl
            << "0 - Exit" << std::endl;
        std::cout << "Waiting for input: ";
        int inputOption;
        std::cin >> inputOption;
        std::cout << std::endl;

        switch (inputOption)
        {
        case 0:
        {
            return 0;
        }
        case 1:
        {
            ExecuteCoroutineTest();
            break;
        }
        case 2:
        {
            ExecuteBookDataExample();
            break;
        }
        case 3:
        {
            ExecuteBookStatusExample();
        }
        }
        std::cout << std::endl;
    }

    return 0;
}