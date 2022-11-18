#pragma once

#include <memory>

#include <grpcpp/channel.h>

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

		// TODO subtask to execute a function

	protected:
		std::unique_ptr<typename grpc_service::Stub> mGrpcStub;
	};
}