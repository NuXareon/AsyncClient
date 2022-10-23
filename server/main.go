package main

import (
	"github.com/gin-gonic/gin"
	"net/http"
	"time"
)

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

func main() {
	router := gin.Default()
	router.GET("/books/available", getAvailableBooks)
	router.GET("/books", getBookData)
	router.GET("/books/collections/book", getCollectionByBook)
	router.GET("/books/publishers/book", getPublisherByBook)

	router.Run("localhost:8080")
}

// curl -X GET "http://localhost:8080/books/available"
func getAvailableBooks(c *gin.Context) {
	var availableBookData []string
	for id := range availableBooks {
		availableBookData = append(availableBookData, id);
	}
	// Fake network conditions
	time.Sleep(500 * time.Millisecond)
	c.IndentedJSON(http.StatusOK, availableBookData)
}

// curl -X GET "http://localhost:8080/books?ids=2&ids=4"
func getBookData(c *gin.Context) {
	ids := c.QueryArray("ids")
	bookExtendedData := make(map[string]bookData)
	for _, id := range ids {
		if book, found := availableBooks[id]; found {
			bookExtendedData[id] = book
		}
	}
	// Fake network conditions
	time.Sleep(500 * time.Millisecond)

	c.IndentedJSON(http.StatusOK, bookExtendedData)
}

// curl -X GET "http://localhost:8080/books/collections/book?ids=2&ids=4&ids=9"
func getCollectionByBook(c *gin.Context) {
	ids := c.QueryArray("ids")
	bookCollection := make(map[string]string)
	for _, id := range ids {
		if book, found := availableBooks[id]; found {
			for collectionName, books := range collections {
				found := false
				for _, collectionBook := range books {
					if collectionBook == book.Title {
						bookCollection[book.Title] = collectionName
						found = true
						break
					}
				}
				if found {
					break
				}
			}
		}
	}
	// Fake network conditions
	time.Sleep(1000 * time.Millisecond)

	c.IndentedJSON(http.StatusOK, bookCollection)
}

// curl -X GET "http://localhost:8080/books/publishers/book?ids=2&ids=4&ids=9"
func getPublisherByBook(c *gin.Context) {
	ids := c.QueryArray("ids")
	bookPublisher := make(map[string]string)
	for _, id := range ids {
		if book, found := availableBooks[id]; found {
			for publisherName, publishedBooks := range publishers {
				found := false
				for _, publishedBook := range publishedBooks {
					if publishedBook == book.Title {
						bookPublisher[book.Title] = publisherName
						found = true
						break
					}
				}
				if found {
					break
				}
			}
		}
	}
	// Fake network conditions
	time.Sleep(1000 * time.Millisecond)

	c.IndentedJSON(http.StatusOK, bookPublisher)
}