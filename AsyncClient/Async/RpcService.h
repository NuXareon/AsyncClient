#pragma once

#include <memory>

#include <grpcpp/channel.h>

template <class TGrpcService>
class RpcService
{
public:
	using grpc_service = typename TGrpcService;

protected:
	RpcService(std::shared_ptr<grpc::Channel> channel) :
		mGrpcStub(grpc_service::NewStub(channel))
	{
	}

	// TODO subtask to execute a function

protected:
	std::unique_ptr<typename grpc_service::Stub> mGrpcStub;
};