package main

type bookData struct {
	Title  	string  `json:"title"`
	Author 	string  `json:"author"`
	Year  	int16	`json:"year"`
}

var availableBooks = map[string]bookData {
	"2" : {"The Two Towers", "J. R. R. Tolkien", 1954},
	"4" : {"The Hobbit", "J. R. R. Tolkien", 1937},
	"7" : {"The Exploits of Sherlock Holmes", "Adrian Conan Doyle and John Dickson Carr", 1954},
	"8" : {"I Am Legend", "Richard Matheson", 1954},
	"9" : {"Kings of the Wyld", "Nicholas Eames", 2017},
}

// All reference should go by id and not name probably, but w/e
var collections = map[string][]string {
	"The Lord Of The Rings" : {"The fellowship of the Ring", "The Two Towers", "The Return of the King"},
	"The Band" : {"Kings of the Wyld", "Bloody Rose"},
}

var publishers = map[string][]string {
	"George Allen and Unwin" : {"The Two Towers", "The Hobbit"},
	"John Murray" : {"The Exploits of Sherlock Holmes"},
	"Gold Medal Books" : {"I Am Legend"},
	"Orbit" : {"Kings of the Wyld"},
}

var locations = []string {
	"Barcelona",
	"Berlin",
	"Paris",
	"Tokio",
	"London",
}