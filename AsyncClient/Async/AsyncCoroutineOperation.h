#pragma once

#include <iostream>

#include "AsyncOperation.h"
#include "Coroutine.h"
#include "../Utils.h"

namespace Async
{
    namespace Private
    {
        template <typename T, typename ReturnType>
        concept FutureType = requires (T a, std::chrono::seconds s)
        {
            { a.wait_for(s) } -> std::same_as<std::future_status>;
            { a.get() } -> std::same_as<ReturnType>;
        };

        // Async::Operation fulfill this requirements
        template<typename T, class ... Args>
        concept OperationType = requires (Args&& ... args)
        {
            typename T::return_type;
            typename T::async_return_type;
            { T::ExecuteOperation(std::forward<Args...>(args)...) } -> FutureType<typename T::async_return_type>;
            // { T::StartOperation(args...) } -> std::same_as<typename T::async_return_type>;
        };

        template <typename T>
        concept Resumable =
            requires (T a) { a.Resume; } ||
            requires (T a) { a.resume; } ||
            requires (T a) { a(); };    // This technically makes lambdas valid inputs too tho..
    }

    template <class AsyncOperation, class ... Args>
    requires Private::OperationType<AsyncOperation, Args...>
    Task<typename AsyncOperation::async_return_type> RunOperation(Args&& ... args)
    {
        using namespace std::chrono_literals;

        DebugLog("Operation Start\n");

        auto asyncResult = AsyncOperation::ExecuteOperation(std::forward<Args...>(args)...);

        while (asyncResult.wait_for(0s) != std::future_status::ready)
        {
            co_await std::suspend_always{};
        }

        DebugLog("Operation Finished\n");

        co_return asyncResult.get();
    }

    namespace Private
    {
        template <class T>
        bool ExecuteRecursiveAsyncTasks(T& currentTask)
        {
            return currentTask.Resume();
        }

        template <class T, class ... TAsyncTasks>
        bool ExecuteRecursiveAsyncTasks(T& currentTask, TAsyncTasks& ... asyncTasks)
        {
            bool finished = currentTask.Resume();

            finished &= Private::ExecuteRecursiveAsyncTasks(std::forward<TAsyncTasks&>(asyncTasks)...);

            return finished;
        }
    }

    template <Private::Resumable ... TAsyncTasks>
    Task<> RunOperationsParallel(TAsyncTasks& ... asyncTasks)
    {
        while (true)
        {
            if (Private::ExecuteRecursiveAsyncTasks(std::forward<TAsyncTasks&>(asyncTasks)...))
                break; // Could do co_return here too

            co_await std::suspend_always{};
        }

        co_return;
    }

}
/*
template <class T, class ... TAsyncTasks>
std::vector<AsyncTaskBase*> GenerateAsyncTasks()
{
    if (sizeof...(TAsyncTasks) > 0)
    {
        GenerateAsyncTasks<TAsyncTasks...>();
    }

    auto operation = RunOperation<T>();

    return {};
}
*/