#pragma once

#include <iostream>
#include <future>
#include "restclient-cpp/restclient.h"
#include "../JSON/json.hpp"

namespace Async
{
	template <class TReturn>
	struct OperationResult
	{
		OperationResult() = default;
		OperationResult(TReturn&& returnValue)
			: mReturnValue(returnValue)
		{
		}
		OperationResult(int responseCode, TReturn&& returnValue)
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
	struct OperationResult<void>
	{
		OperationResult() = default;
		OperationResult(int responseCode)
			: mResponseCode(responseCode)
		{
		}
		int mResponseCode = -1;
	};

	// TODO use Concept to ensure classes
	template <class TChild, typename TReturnType>
	class Operation
	{
	public:
		using return_type = TReturnType;
		using async_return_type = OperationResult<return_type>;

		template <class ... Args>
		static std::future<async_return_type> ExecuteOperation(Args&& ... args)
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
#if _DEBUG
				std::cout << "Response (" << httpResponse.code << ") - " << retunValue << "\n";
#endif
				if (retunValue.is_discarded())
				{
					// Invalid return value - most likely a backend error. Trying to get the json value in this state would result in an exception/crash.
					return ReturnErrorCode(httpResponse.code);
				}

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
}