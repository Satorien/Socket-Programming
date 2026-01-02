all: server client

server: server.c utils.c
	gcc server.c utils.c -o run_server -Wall -Wextra -pedantic -std=c11

client: client.c utils.c
	gcc client.c utils.c -o run_client -Wall -Wextra -pedantic -std=c11

clean:
	rm -f run_server run_client