// CLIENT-SIDE (TCP)
#include <stdio.h> // common C utils
#include <stdlib.h> // other common C utils
#include <sys/types.h> // common types
#include <sys/socket.h> // socket utils
#include <unistd.h> // unix api
#include <netinet/in.h> // protocol utils

int main(void) {
	int network_socket; // file descriptor ID for the network socket
	network_socket = socket(AF_INET, SOCK_STREAM, 0); // ipv4 socket
	// socket(domain, type, protocol);
	// domain is AF_INET (ipv4) - AF_INET6 (ipv6)
	// type is SOCK_STREAM (TCP) - SOCK_DGRAM (UDP)
	// protocol 0 lets O.S. pick default protocol appropriate for params
	if (network_socket < 0) {
		printf("network_socket failed (%d)\n", network_socket);
		exit(EXIT_FAILURE);
	} else {
		printf("%i\n", network_socket);
	}

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	// we are now requesting to hop in the server! this must be accepted by the server itself
	int connection_status = connect(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	// receiving the message from the server
	char server_response[256];
	recv(network_socket, &server_response, sizeof(server_response), 0);

	printf("The server sent the data: %s\n", server_response);

	close(network_socket);
	return 0;
}
