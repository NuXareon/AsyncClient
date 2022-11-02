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
	// TODO: make debug text
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
	auto availableBookIds = co_await StartAsyncCoroutineOperation<GetAvailableBooksOperation>();
	if (!availableBookIds.HasSuccess())
	{
		std::cout << "Error retrieving availabe books: " << availableBookIds.mResponseCode << std::endl;
		co_return;
	}

	auto bookInfo = co_await StartAsyncCoroutineOperation<GetBookInfoOperation>(availableBookIds.mReturnValue);
	if (!bookInfo.HasSuccess())
	{
		std::cout << "Error retrieving book info: " << bookInfo.mResponseCode << std::endl;
		co_return;
	}

	auto& bookData = bookInfo.mReturnValue;
	[[maybe_unused]] auto filteredCount = FilterBookInfoByYear(bookData, year);
	DebugLog("Filtered %zd books not matching the date. \n", filteredCount);
	
	std::vector<std::string> filteredBookIds;
	for (const auto& bookEntry : bookData)
	{
		filteredBookIds.push_back(bookEntry.first);
	}
	auto bookCollection = co_await StartAsyncCoroutineOperation<GetBookCollectionOperation>(filteredBookIds);

	if (!bookCollection.HasSuccess())
	{
		std::cout << "Error retrieving book collection: " << bookCollection.mResponseCode << std::endl;
		co_return;
	}

	DisplayBookData(bookData, bookCollection.mReturnValue);

	co_return;
}

std::size_t DisplayBooksByYearUI::FilterBookInfoByYear(std::map<std::string, BookInfo>& bookData, int year) const
{
	return std::erase_if(bookData, [year](const auto& item)
		{
			return item.second.year != year;
		});
}

void DisplayBooksByYearUI::DisplayBookData(const std::map<std::string, BookInfo>& bookData, const std::map<std::string, std::string>& bookCollection) const
{
	std::cout << bookData.size() << " books foud:" << std::endl;
	for (const auto& book : bookData)
	{
		std::cout << book.second.title << " (" << book.second.year << ")" << " by " << book.second.author;
		const auto it = bookCollection.find(book.second.title);
		if (it != bookCollection.end())
		{
			std::cout << " (Part of '" << it->second << "' collection)";
		}
		std::cout << std::endl;
	}
}