#include "BookStatusService.h"

BookStatusService::BookStatusService(std::shared_ptr<grpc::Channel> channel) 
	: RpcService(channel)
{

}

BookStatusService::rpc_task<BookStatusService::getbookstatus_return> BookStatusService::GetBookStatusTask(std::vector<std::string> ids)
{
	// TODO: make this generic
	std::vector<rpc_task<std::pair<std::string, BookService::BookState>>> tasks;
	for (std::string id : ids)
	{
		auto operation = ExecuteAsTask<rpc_task<std::pair<std::string, BookService::BookState>>>([=]()
			{
				return this->GetBookStatus(id);
			});
		tasks.push_back(std::move(operation));
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

	std::map<std::string, BookService::BookState> returnVal;
	for (const auto& task : tasks)
	{
		if (task.HasResult())
		{
			const auto& result = task.GetResult();
			if (!result.HasSuccess())
			{
				co_return ReturnErrorCode<getbookstatus_return>(result.mResponseCode);
			}

			returnVal.emplace(result.mReturnValue);
		}
	}

	co_return grpc_return_type<BookStatusService::getbookstatus_return>{ grpc::Status::OK, std::move(returnVal) };
}

BookStatusService::grpc_return_type<std::pair<std::string, BookService::BookState>> BookStatusService::GetBookStatus(const std::string& id)
{
	DebugLog("GetBookStatus Start\n");

	grpc::ClientContext clientContext;
	BookService::BookId bookId;
	bookId.set_id(id);
	BookService::BookState responseState;
	grpc::Status status = mGrpcStub->GetBookStatus(&clientContext, bookId, &responseState);
	DebugLog("Rpc status (%d) - %s\n", status.error_code(), status.error_message().c_str());

	if (!status.ok())
	{
		return ReturnErrorCode<std::pair<std::string, BookService::BookState>>(status);
	}

	return { grpc::Status::OK, {id, responseState} };
}

BookStatusService::grpc_return_type<BookStatusService::getbookstatus_return> BookStatusService::GetMultiBookStatus(const std::vector<std::string>& ids)
{
	DebugLog("GetMultiBookStatus Start\n");

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