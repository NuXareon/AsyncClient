#pragma once

#include "RpcService.h"
#include "protobuf/BookLibraryService.grpc.pb.h"
#include "Async/Coroutine.h"

#include "Utils.h"
#include <future>

class BookStatusService : public Async::RpcService<BookService::BookLibrary>
{
public:
	BookStatusService(std::shared_ptr<grpc::Channel> channel);

	using getbookstatus_return = std::map<std::string, BookService::BookState>;
	rpc_task<getbookstatus_return> GetBookStatusTask(std::vector<std::string> ids);

private:
	grpc_return_type<std::pair<std::string, BookService::BookState>> GetBookStatus(const std::string& id);

	// Warning: fires sequential calls for each id, there is no bulking.
	grpc_return_type<getbookstatus_return> GetMultiBookStatus(const std::vector<std::string>& ids);
};