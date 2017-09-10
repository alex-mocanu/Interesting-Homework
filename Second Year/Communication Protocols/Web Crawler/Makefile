build: server.cpp client.cpp utils.h
	g++ server.cpp utils.h -o server
	g++ client.cpp utils.h -o client

build-server: server.cpp utils.h
	g++ -g server.cpp utils.h -o server

build-client: client.cpp utils.h
	g++ client.cpp utils.h -o client

run-server:
	./server -r -e -o serverLog -p 10000

run-client:
	./client -o clientLog -a 127.0.0.1 -p 10000

clean:
	rm -f server client