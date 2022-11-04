#pragma once

#include "AsyncOperation.h"
#include "../BookTypes.h"

class GetAvailableBooksOperation : public AsyncOperation<GetAvailableBooksOperation, std::vector<std::string>>
{
public:
	static async_return_type StartOperation();
};

class GetBookInfoOperation : public AsyncOperation<GetBookInfoOperation, std::map<std::string, BookInfo>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};

class GetBookCollectionOperation : public AsyncOperation<GetBookCollectionOperation, std::map<std::string, std::string>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};

class GetBookPublisherOperation : public AsyncOperation<GetBookPublisherOperation, std::map<std::string, std::string>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};