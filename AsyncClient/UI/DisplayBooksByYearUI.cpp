#include "DisplayBooksByYearUI.h"

// TODO: make paths absolute
#include "../Utils.h"
#include "../Async/BookOperations.h"
#include "../Async/AsyncCoroutineOperation.h"

#include <iostream>

void DisplayBooksByYearUI::Start()
{
	DebugLog("Starting UI\n");
	mFetchBookOperation = FetchBookData(YEAR_TO_FILTER);

	std::cout << "Diplaying data for available books release on " << YEAR_TO_FILTER << std::endl;
}

bool DisplayBooksByYearUI::Tick(long long frameCount)
{
	DebugLog("Ticking UI (%lld)\n", frameCount);
	mFetchBookOperation.Resume();
	return !mFetchBookOperation.IsFinished();
}

void DisplayBooksByYearUI::End()
{
	DebugLog("Closing UI\n");
}

AsyncTask<> DisplayBooksByYearUI::FetchBookData(int year)
{
	// First fetch, function will resume when we have a result from backend (or an error happens)
	auto availableBookIds = co_await StartAsyncCoroutineOperation<GetAvailableBooksOperation>();
	if (!availableBookIds.HasSuccess())
	{
		std::cout << "Error retrieving availabe books: " << availableBookIds.mResponseCode << std::endl;
		co_return;
	}

	// Use the previous value to perform the next query
	auto bookInfo = co_await StartAsyncCoroutineOperation<GetBookInfoOperation>(availableBookIds.mReturnValue);
	if (!bookInfo.HasSuccess())
	{
		std::cout << "Error retrieving book info: " << bookInfo.mResponseCode << std::endl;
		co_return;
	}

	// Transform obtained data
	auto& bookData = bookInfo.mReturnValue;
	[[maybe_unused]] auto filteredCount = FilterBookInfoByYear(bookData, year);
	DebugLog("Filtered %zd books not matching the date. \n", filteredCount);
	
	std::vector<std::string> filteredBookIds;
	for (const auto& bookEntry : bookData)
	{
		filteredBookIds.push_back(bookEntry.first);
	}

	// Run two operations in parallel
	auto getBookCollectionOp = StartAsyncCoroutineOperation<GetBookCollectionOperation>(filteredBookIds);
	auto getBookPublisherOperation = StartAsyncCoroutineOperation<GetBookPublisherOperation>(filteredBookIds);
	co_await ExecuteParallelOperations(getBookCollectionOp, getBookPublisherOperation);

	auto bookCollection = getBookCollectionOp.GetResult();
	auto bookPublisher = getBookPublisherOperation.GetResult();

	if (!bookCollection.HasSuccess())
	{
		std::cout << "Error retrieving book collection: " << bookCollection.mResponseCode << std::endl;
		co_return;
	}
	if (!bookPublisher.HasSuccess())
	{
		std::cout << "Error retrieving book publisher: " << bookPublisher.mResponseCode << std::endl;
		co_return;
	}

	DisplayBookData(bookData, bookCollection.mReturnValue, bookPublisher.mReturnValue);
	
	co_return;
}

std::size_t DisplayBooksByYearUI::FilterBookInfoByYear(std::map<std::string, BookInfo>& bookData, int year) const
{
	return std::erase_if(bookData, [year](const auto& item)
		{
			return item.second.year != year;
		});
}

void DisplayBooksByYearUI::DisplayBookData(const std::map<std::string, BookInfo>& bookData, const std::map<std::string, std::string>& bookCollection, const std::map<std::string, std::string>& bookPublisher) const
{
	std::cout << bookData.size() << " books foud:" << std::endl;
	for (const auto& book : bookData)
	{
		std::cout << book.second.title << " (" << book.second.year << ")" << " by " << book.second.author;
		const auto itCollection = bookCollection.find(book.second.title);
		if (itCollection != bookCollection.end())
		{
			std::cout << " (Part of '" << itCollection->second << "')";
		}
		std::cout << " - Published by: ";
		const auto itPublisher = bookPublisher.find(book.second.title);
		if (itPublisher != bookPublisher.end())
		{
			std::cout << itPublisher->second;
		}
		else
		{
			std::cout << "<No Publisher>";
		}
		std::cout << std::endl;
	}
}