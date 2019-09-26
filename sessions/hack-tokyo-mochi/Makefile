CFLAGS += `pkg-config --cflags thallium`
LDFLAGS += `pkg-config --libs thallium`
CC = g++

all:: client server

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp -std=c++14

client: client.o
	$(CC) client.o -o client $(LDFLAGS) -std=c++14

server.o: server.cpp
	$(CC) $(CFLAGS) -I/home/shuuji3/hack-kyoto-mochi/include -c server.cpp -std=c++14

server: server.o
	$(CC) server.o -o server $(LDFLAGS) -L/home/shuuji3/hack-kyoto-mochi/lib -lkyotocabinet -std=c++14
#	$(CC) server.o -o server  $(LDFLAGS) -I/home/shuuji3/hack-kyoto-mochi/include -L/home/shuuji3/hack-kyoto-mochi/lib -lkyotocabinet -lz -lstdc++ -lrt -lpthread -lm -lc

clean::
	rm -f client server client.o server.o
