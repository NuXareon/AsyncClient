#pragma once

// TODO should clean up includes project wide
#include "../Async/Coroutine.h"
#include <map>
#include <string>

struct BookInfo;

class DisplayBooksByYearUI
{
	// TODO this should be an input
	static constexpr int YEAR_TO_FILTER = 1954;

public:
	void Start();
	bool Tick(long long frameCount);
	void End();

private:
	Async::Task<bool> FetchBookData(int year);
	std::size_t FilterBookInfoByYear(std::map<std::string, BookInfo>& bookData, int year) const;
	void DisplayBookData(const std::map<std::string, BookInfo>& bookData, const std::map<std::string, std::string>& bookCollection, const std::map<std::string, std::string>& bookPublisher) const;

private:
	Async::Task<bool> mFetchBookOperation;
};