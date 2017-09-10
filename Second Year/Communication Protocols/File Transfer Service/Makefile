build: client.cpp server.cpp
	g++ client.cpp -o client
	g++ server.cpp -o server

build-server: server.cpp
	g++ server.cpp -o server

build-client: client.cpp
	g++ client.cpp -o client

run-server:
	./server 10000 users_config shared_files

run-client:
	./client 127.0.0.1 10000

clean:
	rm -f server client
