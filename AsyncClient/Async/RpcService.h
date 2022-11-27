#pragma once

#include <memory>
#include <future>

#include <grpcpp/channel.h>

#include "Coroutine.h"

namespace Async
{
	template <class TReturn>
	struct RpcResult
	{
		RpcResult() = default;
		RpcResult(TReturn&& returnValue)
			: mReturnValue(returnValue)
		{
		}
		RpcResult(const grpc::Status& responseCode, TReturn&& returnValue)
			: mResponseCode(responseCode)
			, mReturnValue(std::forward<TReturn&&>(returnValue))
		{
		}

		bool HasSuccess() const
		{
			return mResponseCode.ok();
		}

		grpc::Status mResponseCode;
		TReturn mReturnValue;
	};

	template <>
	struct RpcResult<void>
	{
		RpcResult() = default;
		RpcResult(const grpc::Status& responseCode)
			: mResponseCode(responseCode)
		{
		}
		grpc::Status mResponseCode;
	};

	template <class TGrpcService>
	class RpcService
	{
	public:
		using grpc_service = typename TGrpcService;

		template <class TReturn>
		using grpc_return_type = RpcResult<TReturn>;

		template <class T>
		using rpc_task = Async::Task<grpc_return_type<T>>;

	protected:
		RpcService(std::shared_ptr<grpc::Channel> channel) :
			mGrpcStub(grpc_service::NewStub(channel))
		{
		}

		template <class TReturn>
		static grpc_return_type<TReturn> ReturnErrorCode(grpc::Status responseCode = grpc::Status::CANCELLED)
		{
			grpc_return_type<TReturn> returnVal;
			returnVal.mResponseCode = responseCode;
			return returnVal;
		}

		// TODO concept to ensure executable and TCallback::Return type == TTask (also deduce type from return?)
		template <class TTask, class TCallback>
		TTask ExecuteAsTask(TCallback callback)
		{
			using namespace std::chrono_literals;

			DebugLog("RPC Start\n");

			auto asyncResult = std::async(std::launch::async, callback);

			while (asyncResult.wait_for(0s) != std::future_status::ready)
			{
				co_await std::suspend_always{};
			}

			DebugLog("RPC Finished\n");

			co_return asyncResult.get();
		}

		template <class TTask, class TInput>
		Async::Task<std::vector<TTask>> ExecuteTasksParallel(const std::vector<TInput>& inputs, std::function<TTask(TInput)> callback)
		{
			std::vector<TTask> tasks;
			for (const auto& input : inputs)
			{
				TTask task = callback(input);
				tasks.push_back(std::move(task));
			}

			while (true)
			{
				bool allFinished = true;
				for (auto& task : tasks)
				{
					allFinished &= task.Resume();
				}

				if (allFinished)
				{
					break;
				}

				co_await std::suspend_always{};
			}

			co_return tasks;
		}

	protected:
		std::unique_ptr<typename grpc_service::Stub> mGrpcStub;
	};
}