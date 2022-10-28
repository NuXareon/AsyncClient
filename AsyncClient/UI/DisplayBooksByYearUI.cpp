#include "DisplayBooksByYearUI.h"

// TODO: make paths absolute
#include "../Utils.h"
#include "../Async/AsyncOperation.h"
#include "../Async/AsyncCoroutineOperation.h"

#include <iostream>

void DisplayBooksByYearUI::Start()
{
	DebugLog("Starting UI\n");
	mFetchBookOperation = FetchBookData(YEAR_TO_FILTER);
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
	//auto bookCollection = co_await StartAsyncCoroutineOperation<GetBookCollectionOperation>(filteredBookIds);
	co_return;
}

std::size_t DisplayBooksByYearUI::FilterBookInfoByYear(std::map<std::string, BookInfo>& bookData, int year) const
{
	return std::erase_if(bookData, [year](const auto& item)
		{
			return item.second.year != year;
		});
}
