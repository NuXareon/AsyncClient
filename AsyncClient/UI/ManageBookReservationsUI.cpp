#include "ManageBookReservationsUI.h"

// REST
#include "Async/AsyncCoroutineOperation.h"
#include "Async/BookOperations.h"

// RPC
#include "Async/BookStatusService.h"

// Etc...
#include "Utils.h"

ManageBookReservationsUI::ManageBookReservationsUI(const std::shared_ptr<BookStatusService>& bookStatusService)
    : mBookStatusService(bookStatusService)
{
}

void ManageBookReservationsUI::Start()
{
    DebugLog("Starting UI\n");
    mProcessBookReservationsTask = ProcessBookReservations();

    std::cout << "Processing book reservations..." << std::endl;
}

bool ManageBookReservationsUI::Tick(long long frameCount)
{
    DebugLog("Ticking UI (%lld)\n", frameCount);

    mProcessBookReservationsTask.Resume();
    if (mProcessBookReservationsTask.HasResult() && mProcessBookReservationsTask.GetResult() == false)
    {
        std::cout << "Failed to process book reservations" << std::endl;
        return false;	// Close the UI
    }
    return !mProcessBookReservationsTask.IsFinished();
}

void ManageBookReservationsUI::End()
{
    DebugLog("Closing UI\n");
}

Async::Task<bool> ManageBookReservationsUI::ProcessBookReservations()
{
    // REST call
    auto availableBookIds = co_await Async::RunOperation<GetAvailableBooksOperation>();
    if (!availableBookIds.HasSuccess())
    {
        std::cout << "Failed to retrieve available books: " << availableBookIds.mResponseCode << std::endl;
        co_return false;
    }

    // Multiple RPC calls in parallel
    auto bookStatusResult = co_await mBookStatusService->GetBookStatusTask(availableBookIds.mReturnValue);
    if (!bookStatusResult.HasSuccess())
    {
        std::cout << "Failed to retrieve book status: " << bookStatusResult.mResponseCode.error_code() << " - " << bookStatusResult.mResponseCode.error_message() << std::endl;
        co_return false;
    }

    for (auto& entry : bookStatusResult.mReturnValue)
    {
        PrintBookState(entry.first, entry.second);
    }

    co_return true;
}

void ManageBookReservationsUI::PrintBookState(std::string_view bookId, const BookService::BookState& book) const
{
    std::cout << bookId << " : " << BookService::BookStateEnum_Name(book.state());

    if (book.state() == BookService::BookStateEnum::Available)
    {
        std::cout << "[";
        const auto locationCount = book.location_size();
        for (std::remove_const_t<decltype(locationCount)> locationIdx = 0; locationIdx < locationCount; ++locationIdx)
        {
            std::cout << " " << book.location(locationIdx);
        }
        std::cout << " ]" << std::endl;
    }
    else if (book.state() == BookService::BookStateEnum::Reserved)
    {
        if (book.has_user())
        {
            std::cout << "[ " << book.user() << " ]" << std::endl;
        }
        else
        {
            std::cout << "* No User *" << std::endl;
        }
    }
}