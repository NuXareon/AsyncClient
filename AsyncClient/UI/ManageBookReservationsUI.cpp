#include "ManageBookReservationsUI.h"

#include "Async/BookStatusService.h"
#include "Utils.h"

ManageBookReservationsUI::ManageBookReservationsUI(const std::shared_ptr<BookStatusService>& bookStatusService)
    : mBookStatusService(bookStatusService)
{
}

void ManageBookReservationsUI::Start()
{
    DebugLog("Starting UI\n");
    mProcessBookReservationsTask = ProcessBookReservations();

    std::cout << "Processing book reservations...";
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
    std::vector<std::string> ids;
    ids.push_back("2");
    ids.push_back("4");
    auto bookStatusResult = co_await mBookStatusService->GetBookStatusTask(ids);

    if (!bookStatusResult.HasSuccess())
    {
        std::cout << "Failed to retrieve book status: " << bookStatusResult.mResponseCode.error_code() << " - " << bookStatusResult.mResponseCode.error_message() << std::endl;
        co_return false;
    }

    for (auto& entry : bookStatusResult.mReturnValue)
    {
        std::cout << entry.first << " : " << BookService::BookStateEnum_Name(entry.second.state()) << std::endl;
    }

    co_return true;
}