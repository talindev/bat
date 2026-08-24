#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define PORT 80

// part of Tristan Hundley's Medium post studies
// executes GET / requests
int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    const char *hostname = argv[1];
    struct hostent *server;

    struct sockaddr_in server_addr;
    int sockfd;

    if (argc != 2 ) {
        fprintf(stderr, "Error: Only one hostname is accepted for '%s <hostname>'\n", argv[0]);
        return 1;
    }

    if ((server = gethostbyname(hostname)) == NULL) {
        fprintf(stderr, "Error: This host does not exist\n");
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Could not create socket\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error: Could not connect to server, please check hostname\n");
        close(sockfd);
        return 1;
    }

    snprintf(buffer, sizeof(buffer), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("Error: Could not send request\n");
        close(sockfd);
        return 1;
    }

    int received;
    while ((received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[received] = '\0';
        printf("\x1b[38;5;154m"); // text color green
        printf("%s", buffer);
        printf("\x1b[0m");
    }
    if (received < 0) {
        perror("Error: Could not receive response\n");
    }

    if (close(sockfd) < 0) {
        perror("Error: Could not close socket\n");
        return 1;
    }
}
