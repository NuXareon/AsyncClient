#include "DisplayBooksByYearUI.h"

// TODO: make paths absolute
#include "../Utils.h"
#include "../Async/BookOperations.h"
#include "../Async/AsyncCoroutineOperation.h"

#include <iostream>

void DisplayBooksByYearUI::Start()
{
	DebugLog("Starting UI\n");
	mFetchBookTask = FetchBookData(YEAR_TO_FILTER);

	std::cout << "Diplaying data for available books released on " << YEAR_TO_FILTER << std::endl;
}

bool DisplayBooksByYearUI::Tick(long long frameCount)
{
	DebugLog("Ticking UI (%lld)\n", frameCount);
	mFetchBookTask.Resume();
	if (mFetchBookTask.HasResult() && mFetchBookTask.GetResult() == false)
	{
		std::cout << "Failed to display book data" << std::endl;
		return false;	// Close the UI
	}
	return !mFetchBookTask.IsFinished();
}

void DisplayBooksByYearUI::End()
{
	DebugLog("Closing UI\n");
}

Async::Task<bool> DisplayBooksByYearUI::FetchBookData(int year)
{
	// First fetch, function will resume when we have a result from backend (or an error happens)
	auto availableBookIds = co_await Async::RunOperation<GetAvailableBooksOperation>();
	if (!availableBookIds.HasSuccess())
	{
		std::cout << "Error retrieving availabe books: " << availableBookIds.mResponseCode << std::endl;
		co_return false;
	}

	// Use the previous value to perform the next query
	auto bookInfo = co_await Async::RunOperation<GetBookInfoOperation>(availableBookIds.mReturnValue);
	if (!bookInfo.HasSuccess())
	{
		std::cout << "Error retrieving book info: " << bookInfo.mResponseCode << std::endl;
		co_return false;
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
	auto getBookCollectionOp = Async::RunOperation<GetBookCollectionOperation>(filteredBookIds);
	auto getBookPublisherOperation = Async::RunOperation<GetBookPublisherOperation>(filteredBookIds);
	co_await Async::RunOperationsParallel(getBookCollectionOp, getBookPublisherOperation);

	auto bookCollection = getBookCollectionOp.GetResult();
	auto bookPublisher = getBookPublisherOperation.GetResult();

	if (!bookCollection.HasSuccess())
	{
		std::cout << "Error retrieving book collection: " << bookCollection.mResponseCode << std::endl;
		co_return false;
	}
	if (!bookPublisher.HasSuccess())
	{
		std::cout << "Error retrieving book publisher: " << bookPublisher.mResponseCode << std::endl;
		co_return false;
	}

	DisplayBookData(bookData, bookCollection.mReturnValue, bookPublisher.mReturnValue);
	
	co_return true;
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