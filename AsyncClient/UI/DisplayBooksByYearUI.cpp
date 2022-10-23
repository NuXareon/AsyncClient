#include "DisplayBooksByYearUI.h"

// TODO: make paths absolute
#include "../Utils.h"
#include "../Async/AsyncOperation.h"
#include "../Async/AsyncCoroutineOperation.h"

#include <iostream>

void DisplayBooksByYearUI::Start()
{
	DebugLog("Starting UI\n");
	mFetchBookOperation = FetchBookData();
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

AsyncTask<> DisplayBooksByYearUI::FetchBookData()
{
	auto availableBookIds = co_await StartAsyncCoroutineOperation<GetAvailableBooksOperation>();
	if (!availableBookIds.HasSuccess())
	{
		std::cout << "Error retrieving availabe books: " << availableBookIds.mResponseCode << std::endl;
		co_return;
	}
	auto bookInfo = co_await StartAsyncCoroutineOperation<GetBookInfoOperation>(availableBookIds.mReturnValue);
	co_return;
}
