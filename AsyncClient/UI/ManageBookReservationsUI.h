#pragma once

#include <memory>
#include <string_view>

#include "Async/Coroutine.h"

class BookStatusService;

namespace BookService
{
	class BookState;
}

class ManageBookReservationsUI
{
	// TODO this should be an input
	//static constexpr int YEAR_TO_FILTER = 1954;

public:
	ManageBookReservationsUI(const std::shared_ptr<BookStatusService>& bookStatusService);

	void Start();
	bool Tick(long long frameCount);
	void End();

private:
	Async::Task<bool> ProcessBookReservations();

	void PrintBookState(std::string_view bookId, const BookService::BookState& book) const;
	//std::size_t FilterBookInfoByYear(std::map<std::string, BookInfo>& bookData, int year) const;
	//void DisplayBookData(const std::map<std::string, BookInfo>& bookData, const std::map<std::string, std::string>& bookCollection, const std::map<std::string, std::string>& bookPublisher) const;

private:
	std::shared_ptr<BookStatusService> mBookStatusService;
	Async::Task<bool> mProcessBookReservationsTask;
};