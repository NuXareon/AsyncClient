#pragma once

#include <coroutine>
#include <optional>

#include "../Utils.h"

template <class T> struct PromiseAsyncOperation;
template <class T> struct AsyncTask;
struct AsyncTaskBase;

/// Awaitable ///

// We need to separate this to allow transformations for different promise return types
template <class TReturnValue, typename TPromise>
struct AwaitableAsyncOperationBase
{
	AwaitableAsyncOperationBase(AsyncTask<TReturnValue>&& subTask)
		: mTask(std::move(subTask))
	{

	}

	// Support co_await
	bool await_ready()
	{
		return mTask.IsFinished();
	}

	// true = suspend, false = resume
	bool await_suspend(std::coroutine_handle<TPromise> currentTaskHandle)
	{
		// Set up and execute subtask when calling co_await on a AsyncTask
		// TODO We should probably use share pointers for safety
		auto* currentTask = currentTaskHandle.promise().mAsyncTask;
		auto* subTask = &mTask;

		currentTask->mSubTask = static_cast<AsyncTaskBase*>(subTask);

		// Execute inital call to subtask
		if (subTask->Resume())
		{
			// Subtask done, resume main task
			currentTask->mSubTask = nullptr;
			return false;
		}

		// Subtask not done, suspend main task
		return true;
	}

	AsyncTask<TReturnValue> mTask;
};

template <class TReturnValue, typename TPromise>
struct AwaitableAsyncOperation : public AwaitableAsyncOperationBase<TReturnValue, TPromise>
{
	AwaitableAsyncOperation(AsyncTask<TReturnValue>&& subTask)
		: AwaitableAsyncOperationBase<TReturnValue, TPromise>(std::move(subTask))
	{

	}

	// Initially had a template specialization for this, but if constexpr is cleaner.
	TReturnValue await_resume()
	{
		// This will directly return the return value when resuming from a co_await from an AsyncTask.
		// Another option would be to return the promise object.
		if constexpr (std::is_void<TReturnValue>::value)
		{
			return;
		}
		else
		{
			return this->mTask.GetResult();
		}
	}
};

/// Promise ///

template <class TPromise, class TReturn>
struct PromiseAsyncOperationBase
{
	// Standard promise config
	AsyncTask<typename TReturn> get_return_object()
	{
		return { std::coroutine_handle<TPromise>::from_promise(*static_cast<TPromise*>(this)) };
	}
	std::suspend_always initial_suspend() noexcept { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	void unhandled_exception() {}

	// TODO should we also support non-movable tasks?
	// Support co_await of different return types
	template<class TSubReturnValue>
	auto await_transform(AsyncTask<TSubReturnValue>&& subTask)
	{
		return AwaitableAsyncOperation<TSubReturnValue, TPromise>(std::move(subTask));
	}

	auto await_transform(std::suspend_always suspend)
	{
		return suspend;
	}

	AsyncTask<TReturn>* mAsyncTask = nullptr;	// Pointer to the task using this promise.
};

template <class TReturnValue>
struct PromiseAsyncOperation : public PromiseAsyncOperationBase<PromiseAsyncOperation<TReturnValue>, TReturnValue>
{
	/*
	// TODO Support co_yield
	template<std::convertible_to<ReturnValue> From> // C++20 concept
	std::suspend_always yield_value(From&& from)
	{
		mReturnValue = std::forward<From>(from);
		mHasResult = true;
		return {};
	}
	*/

	// Support co_return
	template<std::convertible_to<TReturnValue> TFrom> // C++20 concept
	void return_value(TFrom&& from)
	{
		this->mAsyncTask->mResult = std::forward<TFrom>(from);
	}
};

// Specialization to allow tasks with no return type
template <>
struct PromiseAsyncOperation<void> : public PromiseAsyncOperationBase<PromiseAsyncOperation<void>, void>
{
	// Support co_return
	void return_void() {}
};

/// Task ///

struct AsyncTaskBase
{
	AsyncTaskBase() = default;
	AsyncTaskBase& operator= (AsyncTaskBase&& other) noexcept
	{
		mHandle = std::move(other.mHandle);
		mSubTask = std::move(other.mSubTask);

		// We need to invalidate the original handle to ensure that we call the coroutine destructor only once
		other.mHandle = nullptr;
		other.mSubTask = nullptr;

		return *this;
	}

	AsyncTaskBase(std::coroutine_handle<> h)
		: mHandle(h)
	{

	}

	AsyncTaskBase(AsyncTaskBase&& task) noexcept
		: mHandle(std::move(task.mHandle))
		, mSubTask(std::move(task.mSubTask))
	{
		// We need to invalidate the original handle to ensure that we call the coroutine destructor only once
		task.mHandle = nullptr;
		task.mSubTask = nullptr;
	}

	// This should also kill subtasks? (virtual?)
	virtual ~AsyncTaskBase()
	{
		// This can be false when we move a coroutine (i.e. on an awaitable)
		if (mHandle && mHandle.done())
		{
			mHandle.destroy();
		}
	}

	bool IsFinished() const
	{
		return mHandle.done();
	}

	// True if task is finished, otherwise false
	bool Resume()
	{
		if (mSubTask)
		{
			if (!mSubTask->Resume())
			{
				return false;
			}
			mSubTask = nullptr;
		}

		if (mHandle && !mHandle.done())
		{
			mHandle.resume();
			return IsFinished();
		}

		return true;
	}

	bool operator()()
	{
		return Resume();
	}

	// "Strong" handle, will destroy couroutine when tasks is destroy. Need to ensure we only have one.
	std::coroutine_handle<> mHandle;
	AsyncTaskBase* mSubTask = nullptr;
};

template <class TReturnValue = void>
struct AsyncTask : public AsyncTaskBase
{
	using promise_type = PromiseAsyncOperation<TReturnValue>;
	using handle_type = std::coroutine_handle<promise_type>;

	// TODO: This should move the object correctly
	AsyncTask() = default;
	AsyncTask& operator= (AsyncTask&& other) noexcept
	{
		AsyncTaskBase::operator=(std::move(other));

		mHandleWithPromise = std::move(other.mHandleWithPromise);
		mResult = std::move(other.mResult);

		mHandleWithPromise.promise().mAsyncTask = this;
		return *this;
	};

	AsyncTask(handle_type h)
		: AsyncTaskBase(h)
	{
		h.promise().mAsyncTask = this;
		mHandleWithPromise = h;
	}

	AsyncTask(AsyncTask&& other) noexcept
		: AsyncTaskBase(std::move(other))
		, mHandleWithPromise(std::move(other.mHandleWithPromise))
		, mResult(std::move(other.mResult))
	{
		// When moving the async task into the awaitable, we need to update the address stored in the promise.
		// TODO This is kinda ugly since we keep two handles though, there must be a better way.
		mHandleWithPromise.promise().mAsyncTask = this;
	}

	bool HasResult() const
	{
		return mResult.has_value();
	}

	TReturnValue GetResult() const
	{
		ValidateLog(HasResult(), "No result available from coroutine.");
		//return *mResult; // Faster, but could crash. Ideally we want to report if the value is not ready somehow.
		return mResult.value_or(TReturnValue{});
	}

	// "Weak" handle, i.e. doesn't destroy couroutine when out of scope
	handle_type mHandleWithPromise;	// Useful for accessing the promise on a derived task
	std::optional<TReturnValue> mResult;
};

template <>
struct AsyncTask<void> : public AsyncTaskBase
{
	using promise_type = PromiseAsyncOperation<void>;
	using handle_type = std::coroutine_handle<promise_type>;

	AsyncTask() = default;
	AsyncTask& operator= (AsyncTask&& other) noexcept
	{
		AsyncTaskBase::operator=(std::move(other));
		mHandleWithPromise = std::move(other.mHandleWithPromise);
		mHandleWithPromise.promise().mAsyncTask = this;
		return *this;
	};

	AsyncTask(handle_type h)
		: AsyncTaskBase(h)
	{
		h.promise().mAsyncTask = this;
		mHandleWithPromise = h;
	}

	AsyncTask(AsyncTask&& other) noexcept
		: AsyncTaskBase(std::move(other))
		, mHandleWithPromise(std::move(other.mHandleWithPromise))
	{
		// When moving the async task into the awaitable, we need to update the address stored in the promise.
		// TODO This is kinda ugly since we keep two handles though, there must be a better way.
		mHandleWithPromise.promise().mAsyncTask = this;
	}

	// "Weak" handle, i.e. doesn't destroy couroutine when out of scope
	handle_type mHandleWithPromise;	// Useful for accessing the promise on a derived task
};