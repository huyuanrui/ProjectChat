server:
	gcc server.c -I./Include -o server -lpthread -lmysqlclient -L/usr/include/
client:
	gcc client.c -I./Include -o client -lpthread
clean:
	rm server;
	rm client;
