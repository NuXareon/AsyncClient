#pragma once

#include "AsyncOperation.h"
#include "../BookTypes.h"

class GetAvailableBooksOperation : public Async::Operation<GetAvailableBooksOperation, std::vector<std::string>>
{
public:
	static async_return_type StartOperation();
};

class GetBookInfoOperation : public Async::Operation<GetBookInfoOperation, std::map<std::string, BookInfo>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};

class GetBookCollectionOperation : public Async::Operation<GetBookCollectionOperation, std::map<std::string, std::string>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};

class GetBookPublisherOperation : public Async::Operation<GetBookPublisherOperation, std::map<std::string, std::string>>
{
public:
	static async_return_type StartOperation(const std::vector<std::string>& bookIds);
};