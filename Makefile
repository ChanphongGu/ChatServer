all: server client

server: server.cpp http_connection.o bloom_filter.o
	g++ -o server server.cpp  http_connection.o bloom_filter.o -lmysqlclient -lpthread -lhiredis

http_connection.o: http_connection.cpp
	g++ -c http_connection.cpp -lmysqlclient -lpthread -lhiredis

bloom_filter.o: bloom_filter.cpp
	g++ -c bloom_filter.cpp -lmysqlclient -lpthread -lhiredis

client: client.cpp
	g++ -o client client.cpp -lpthread

clean: 
	rm -rf server
	rm -rf client
	rm -rf *.o
	rm -rf *.txt