#pragma once

#include <iostream>

#include "AsyncOperation.h"
#include "Coroutine.h"
#include "../Utils.h"

namespace Async
{
    template <class AsyncOperation, class ... Args>
    Task<typename AsyncOperation::async_return_type> RunOperation(Args ... args)
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

    template <class ... TAsyncTasks>
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