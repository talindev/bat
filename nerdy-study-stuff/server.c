// SERVER-SIDE (TCP)
#include <stdio.h> // common C utils
#include <stdlib.h> // other common C utils
#include <sys/types.h> // common types
#include <sys/socket.h> // socket utils
#include <unistd.h> // unix api
#include <netinet/in.h> // protocol utils

int main(void) {
	int server_socket; // file descriptor ID for the server socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0); // ipv4 socket
        // socket(domain, type, protocol);
        // domain is AF_INET (ipv4) - AF_INET6 (ipv6)
        // type is SOCK_STREAM (TCP) - SOCK_DGRAM (UDP)
        // protocol 0 lets O.S. pick default protocol appropriate for params
        if (server_socket < 0) {
                printf("server_socket failed (%d)\n", server_socket);
                exit(EXIT_FAILURE);
        } else {
                printf("%i\n", server_socket);
        }

	// SOCKETS TALK TO OTHER SOCKETS!
	// this means the network_socket from client will connect to the server_socket from server
	
	// defining the address of our server
	// this is the point where client will look for to connect!
	// socket address = IP + Port
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; // family is ipv4
	server_address.sin_port = htons(9002); // port is 9002 (htons maps the number to usable port)
	server_address.sin_addr.s_addr = INADDR_ANY; // address is any (computer picks one)
	
	// binding the server_socket to the address, so the client will find the socket there
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	// turning the socket into a listening socket - now other clients can connect - via listen
	listen(server_socket, 5); // 5 = maximum number of clients that can wait at once to join a server
	
	// we will allow the connection request pass through with accept()
	int client_socket = accept(server_socket, NULL, NULL);

	// sending the message!
	char server_message[256] = "You have reached the server";
	send(client_socket, server_message, sizeof(server_message), 0);

	close(server_socket);
	close(client_socket);

	return 0;
}
