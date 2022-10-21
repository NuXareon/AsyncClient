#pragma once

#include <iostream>
#include <future>
#include "restclient-cpp/restclient.h"
#include "../JSON/json.hpp"

struct AsyncOpVoidReturnType {};

template <class TReturn>
struct AsyncOperationResult
{
	AsyncOperationResult() = default;
	AsyncOperationResult(TReturn&& returnValue)
		: mReturnValue(returnValue)
	{
	}
	AsyncOperationResult(int responseCode, TReturn&& returnValue)
		: mResponseCode(responseCode)
		, mReturnValue(returnValue)
	{
	}

	bool HasSuccess() const
	{
		return mResponseCode == 0 || mResponseCode == 200;
	}

	int mResponseCode = -1;
	TReturn mReturnValue;
};

template <>
struct AsyncOperationResult<void>
{
	AsyncOperationResult() = default;
	AsyncOperationResult(int responseCode)
		: mResponseCode(responseCode)
	{
	}
	int mResponseCode = -1;
};

// TODO use Concept to ensure classes
template <class TChild, typename TReturnType>
class AsyncOperation
{
public:
	using return_type = TReturnType;
	using async_return_type = AsyncOperationResult<return_type>;

	template <class ... Args>
	static std::future<async_return_type> ExecuteOperation(Args ... args)
	{
		return std::async(std::launch::async, [=]()
			{
				return TChild::StartOperation(args...);
			});
	}

protected:
	static async_return_type ParseReturnValue(const RestClient::Response& httpResponse)
	{
		if constexpr (std::is_void_v<return_type>)
		{
			return async_return_type{ httpResponse.code };
		}
		else
		{
			nlohmann::json retunValue = nlohmann::json::parse(httpResponse.body, nullptr, false);
			std::cout << retunValue << "\n";
			//DebugLog("Response code: %d\n", httpResponse.code);
			return async_return_type{ httpResponse.code, retunValue.get<return_type>() };
		}
	}

	static async_return_type ReturnErrorCode(int errorCode = -1)
	{
		async_return_type returnVal;
		returnVal.mResponseCode = errorCode;
		return returnVal;
	}
};

// TODO move all of this to their own files..
class TestOperation1 : public AsyncOperation<TestOperation1, int>
{
public:

	static async_return_type StartOperation();
};

class TestOperation2 : public AsyncOperation<TestOperation2, float>
{
public:

	static async_return_type StartOperation(float increment);
};

class GetAvailableBooksOperation : public AsyncOperation<GetAvailableBooksOperation, std::vector<std::string>>
{
public:
	static async_return_type StartOperation();
};

// TODO move this somewhere more relevant
struct BookInfo
{
	std::string title;
	std::string author;
	int year;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(BookInfo, title, author, year);
};

class GetBookInfoOperation : public AsyncOperation<GetBookInfoOperation, std::map<std::string, BookInfo>>
{
public:
	static async_return_type StartOperation(std::vector<std::string> bookIds);
};