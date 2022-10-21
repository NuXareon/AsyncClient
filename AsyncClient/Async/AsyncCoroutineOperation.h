#pragma once

#include <iostream>

#include "AsyncOperation.h"
#include "Coroutine.h"
#include "../Utils.h"

template <class T>
class OnlineAsyncOperation : AsyncOperation<T, bool>
{

};

template <class AsyncOperation, class ... Args>
AsyncTask<typename AsyncOperation::async_return_type> StartAsyncCoroutineOperation(Args ... args)
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

template <class T>
bool ExecuteRecursiveAsyncTasks(T& currentTask)
{
    return currentTask.Resume();
}

template <class T, class ... TAsyncTasks>
bool ExecuteRecursiveAsyncTasks(T& currentTask, TAsyncTasks& ... asyncTasks)
{
    bool finished = currentTask.Resume();

    finished &= ExecuteRecursiveAsyncTasks(std::forward<TAsyncTasks&>(asyncTasks)...);

    return finished;
}

template <class ... TAsyncTasks>
AsyncTask<> ExecuteParallelOperations(TAsyncTasks& ... asyncTasks)
{
    while (true)
    {
        if (ExecuteRecursiveAsyncTasks(std::forward<TAsyncTasks&>(asyncTasks)...))
            break; // Could do co_return here too

        co_await std::suspend_always{};
    }

    co_return;
}

/*
template <class T, class ... TAsyncTasks>
std::vector<AsyncTaskBase*> GenerateAsyncTasks()
{
    if (sizeof...(TAsyncTasks) > 0)
    {
        GenerateAsyncTasks<TAsyncTasks...>();
    }

    auto operation = StartAsyncCoroutineOperation<T>();

    return {};
}
*/