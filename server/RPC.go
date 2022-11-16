package main

import (
	"google.golang.org/grpc"
	//"github.com/golang/protobuf/proto"
	"context"
	"log"
	"net"

	pb "example.com/server/protobuff/bookservice"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type bookLibraryServer struct {
	pb.UnimplementedBookLibraryServer

	bookStatus map[string]pb.BookStateEnum
	bookLocation map[string]string;	// TODO we should have several localtions for available books
	bookUsers map[string]string;
}

func (s *bookLibraryServer) GetBookStatus(ctx context.Context, bookId *pb.BookId) (*pb.BookState, error) {
	if bookStatus, found := s.bookStatus[bookId.Id]; found {
		if (bookStatus == pb.BookStateEnum_Unavailable) {
			return &pb.BookState{State: pb.BookStateEnum_Unavailable}, nil
		} else if (bookStatus == pb.BookStateEnum_Reserved) {
			if bookUser, found := s.bookUsers[bookId.Id]; found {
				return &pb.BookState{State: pb.BookStateEnum_Reserved, User: &bookUser}, nil
			}
			return &pb.BookState{State: pb.BookStateEnum_Reserved}, status.Errorf(codes.NotFound, "Book User not found.")
		} else if (bookStatus == pb.BookStateEnum_Available) {
			if bookLocation, found := s.bookLocation[bookId.Id]; found {
				return &pb.BookState{State: pb.BookStateEnum_Available, Location: []string{bookLocation}}, nil
			}
			return &pb.BookState{State: pb.BookStateEnum_Available}, status.Errorf(codes.NotFound, "Book Location not found.") 
		}
		return &pb.BookState{}, status.Errorf(codes.Unknown, "Invalid book state.")
	}

	return &pb.BookState{}, status.Errorf(codes.NotFound, "Book Id not found.")
}

func initBookLabraryService() *bookLibraryServer {
	s := &bookLibraryServer{} // TODO init with data
	return s
}

func InitializeRpcServer() {
	lis, err := net.Listen("tcp", "localhost:50051")
	if err != nil {
		log.Fatalf("RPC server failed to listen: %v", err)
	}
	
	var opts []grpc.ServerOption
	grpcServer := grpc.NewServer(opts...)
	pb.RegisterBookLibraryServer(grpcServer, initBookLabraryService())
	grpcServer.Serve(lis);
}