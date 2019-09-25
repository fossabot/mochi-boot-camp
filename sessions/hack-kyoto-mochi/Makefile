CFLAGS += `pkg-config --cflags margo`
LDFLAGS += `pkg-config --libs margo`

all:: client server

client.o: client.c types.h
	$(CC) $(CFLAGS) -c client.c

client: client.o
	$(CC) client.o -o client  $(LDFLAGS)

server.o: server.c types.h
	$(CC) $(CFLAGS) -I/home/shuuji3/hack-kyoto-mochi/include -c server.c

server: server.o
	$(CC) server.o -o server  $(LDFLAGS) -L/home/shuuji3/hack-kyoto-mochi/lib -lkyotocabinet
#	$(CC) server.o -o server  $(LDFLAGS) -I/home/shuuji3/hack-kyoto-mochi/include -L/home/shuuji3/hack-kyoto-mochi/lib -lkyotocabinet -lz -lstdc++ -lrt -lpthread -lm -lc

clean::
	rm -f client server client.o server.o
