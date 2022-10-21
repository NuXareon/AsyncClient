#pragma once

#include "../Async/Coroutine.h"

class DisplayBooksByYearUI
{
public:
	void Start();
	bool Tick(long long frameCount);
	void End();

private:
	AsyncTask<void> FetchBookData();

private:
	AsyncTask<void> mFetchBookOperation;
};