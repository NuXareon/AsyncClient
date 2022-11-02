#include "BookOperations.h"

#include "../Utils.h"

GetAvailableBooksOperation::async_return_type GetAvailableBooksOperation::StartOperation()
{
	DebugLog("GetAvailableBooksOperation Start\n");
	RestClient::Response r = RestClient::get("http://localhost:8080/books/available");
	return ParseReturnValue(r);
}

GetBookInfoOperation::async_return_type GetBookInfoOperation::StartOperation(const std::vector<std::string>& bookIds)
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

GetBookCollectionOperation::async_return_type GetBookCollectionOperation::StartOperation(const std::vector<std::string>& bookIds)
{
	DebugLog("GetBookCollection Start\n");
	if (bookIds.empty())
	{
		DebugLog("Empty book list found when calling GetBookCollection, canceling operation.");
		return ReturnErrorCode();
	}

	std::string query = "http://localhost:8080/books/collections/book?";
	for (const std::string& id : bookIds)
	{
		query += "ids=" + id + "&";
	}

	RestClient::Response r = RestClient::get(query);
	return ParseReturnValue(r);
}
