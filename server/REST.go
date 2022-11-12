package main

import (
	"github.com/gin-gonic/gin"
	//"google.golang.org/grpc"
	"net/http"
	"time"
)

func InitializeRestServer() {
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