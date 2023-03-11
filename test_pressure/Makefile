all: test_client add_count

test_client: test_client.cpp
	g++ -o test_client test_client.cpp -lpthread

add_count: add_count.cpp
	g++ -o add add_count.cpp -lmysqlclient

clean: 
	rm -rf test_client
	rm -rf add_count
	rm -rf *.o
