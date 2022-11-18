#pragma once

#include "RpcService.h"
#include "protobuf/BookLibraryService.grpc.pb.h"

class BookStatusService : public Async::RpcService<BookService::BookLibrary>
{
public:
	BookStatusService(std::shared_ptr<grpc::Channel> channel)
		: RpcService(channel)
	{

	}

	//::grpc::Status GetBookStatus(::grpc::ClientContext* context, const ::BookService::BookId& request, ::BookService::BookState* response) override;
	using getbookstatus_return = std::map<std::string, BookService::BookState>;
	grpc_return_type<getbookstatus_return> GetBookStatus(const std::vector<std::string>& ids)
	{
		// TODO: fire this ops in parallel
		DebugLog("GetBookStatus Start\n");

		if (ids.empty())
		{
			DebugLog("Invalid parameters, cancelling call\n");
			return ReturnErrorCode<getbookstatus_return>();
		}

		std::map<std::string, BookService::BookState> result;
		for (const auto& id : ids)
		{
			grpc::ClientContext clientContext;
			BookService::BookId bookId;
			bookId.set_id(id);
			BookService::BookState responseState;
			grpc::Status status = mGrpcStub->GetBookStatus(&clientContext, bookId, &responseState);
			DebugLog("Rpc status (%d) - %s\n", status.error_code(), status.error_message().c_str());

			if (!status.ok())
			{
				return ReturnErrorCode<getbookstatus_return>(status);
			}
			
			result.emplace(id, responseState);
		}

		return { grpc::Status::OK, std::move(result) };
	}

private:
};