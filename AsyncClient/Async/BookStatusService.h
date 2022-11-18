#pragma once

#include "RpcService.h"
#include "protobuf/BookLibraryService.grpc.pb.h"

class BookStatusService : public RpcService<BookService::BookLibrary>
{
public:
	BookStatusService(std::shared_ptr<grpc::Channel> channel)
		: RpcService(channel)
	{

	}
};