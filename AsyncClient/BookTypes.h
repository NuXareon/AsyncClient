#pragma once

struct BookInfo
{
	std::string title;
	std::string author;
	int year;

	// Serialization
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(BookInfo, title, author, year);
};