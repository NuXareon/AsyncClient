#include "AsyncOperation.h"

#include "../Utils.h"

#include <iostream>

TestOperation1::async_return_type TestOperation1::StartOperation()
{
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(2s);

	return 1;
}

TestOperation2::async_return_type TestOperation2::StartOperation(float increment)
{
	using namespace std::chrono_literals;

	std::this_thread::sleep_for(2s);

	return 2.f + increment;
}

GetAvailableBooksOperation::async_return_type GetAvailableBooksOperation::StartOperation()
{
	DebugLog("GetAvailableBooksOperation Start\n");
	RestClient::Response r = RestClient::get("http://localhost:8080/books/available");
	return ParseReturnValue(r);
}

GetBookInfoOperation::async_return_type GetBookInfoOperation::StartOperation(std::vector<std::string> bookIds)
{
	DebugLog("GetBookInfoOperation Start\n");
	if (bookIds.empty())
	{
		DebugLog("Empty book list found when calling GetBookInfoOperation, canceling operation.");
		return ReturnErrorCode();
	}

	std::string query = "http://localhost:8080/books?";
	for (const std::string& id : bookIds)
	{
		query += "ids=" + id + "&";
	}

	RestClient::Response r = RestClient::get(query);
	return ParseReturnValue(r);
}