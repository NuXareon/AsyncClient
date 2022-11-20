package main

func main() {
	go InitializeRestServer()
	InitializeRpcServer()
}