
all: server client


server: server.c
	gcc $^ -o $@ -lpthread -levent -g


client:client.c
	gcc $^ -o $@ -lpthread -g


clean:
	rm -rf server client
