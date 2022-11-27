#include "BookStatusService.h"

BookStatusService::BookStatusService(std::shared_ptr<grpc::Channel> channel) 
	: RpcService(channel)
{

}

BookStatusService::rpc_task<BookStatusService::getbookstatus_return> BookStatusService::GetBookStatusTask(std::vector<std::string> ids)
{
	using getbookstatus_task_type = rpc_task<std::pair<std::string, BookService::BookState>>;
	auto tasks = co_await ExecuteTasksParallel<getbookstatus_task_type, std::string>(ids, [this](const std::string& id)
		{
			return ExecuteAsTask<getbookstatus_task_type>([=]()
				{
					return this->GetBookStatus(id);
				});
		});

	std::map<std::string, BookService::BookState> returnVal;
	for (auto& task : tasks)
	{
		AssertLog(task.HasResult(), "Error: Task has no result. This should not happen.");

		if (task.HasResult())
		{
			auto result = task.ConsumeResult();
			if (!result.HasSuccess())
			{
				co_return ReturnErrorCode<getbookstatus_return>(result.mResponseCode);
			}

			returnVal.emplace(std::move(result.mReturnValue));
		}
		else
		{
			DebugLog("Task has no result");
			co_return ReturnErrorCode<getbookstatus_return>();
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