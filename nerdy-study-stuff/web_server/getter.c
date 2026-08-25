#include <_stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 4096

int get_socket(char *hostname, in_port_t port) {
    struct hostent *server;
    struct sockaddr_in server_addr;
    int sockfd;
    int connection;

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error: This host does not exist\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error: Could not create socket\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    connection = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connection < 0) {
        fprintf(stderr, "Error: Could not connect to server\n");
        return 1;
    }

    return sockfd;
}

char *get_protocol(char *url) {
    char *protocol = strstr(url, ":");
    if (!protocol) return NULL;

    int protocol_len = protocol - url;
    char *result = (char *)malloc(protocol_len + 1);
    if (!result) return NULL;

    strncpy(result, url, protocol_len);
    result[protocol_len] = '\0';
    return result;
}

char *get_hostname(char *url) {
    char *protocol = get_protocol(url);
    if (!protocol) return NULL;

    char *domain_start = url + strlen(protocol) + 3;
    char *slash = strstr(domain_start, "/");

    int result_len;
    if (slash != NULL) { // route found
        result_len = slash - domain_start;
    } else { // no route
        result_len = strlen(domain_start);
    }

    char *result = (char *)malloc(result_len + 1);
    if (!result) {
        free(protocol);
        return NULL;
    }

    strncpy(result, domain_start, result_len);
    result[result_len] = '\0';
    free(protocol);
    return result;
}

char *get_route(char *url) {
    char *protocol = get_protocol(url);
    if (!protocol) return NULL;

    char *hostname = get_hostname(url);
    if (!hostname) {
        free(protocol);
        return NULL;
    }

    char *route_start = url + strlen(protocol) + 3 + strlen(hostname);
    if (*route_start == '\0') { // empty route ("/")
        free(protocol);
        free(hostname);
        return NULL;
    }

    int result_len = strlen(route_start);
    char *result = (char *)malloc(result_len + 1);
    if (!result) {
        free(protocol);
        free(hostname);
        return NULL;
    }

    strncpy(result, route_start, result_len);
    result[result_len] = '\0';
    free(protocol);
    free(hostname);
    return result;
}

// part of Tristan Hundley's Medium post studies
// executes GET / requests
int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    char *hostname = argv[1];
    struct hostent *server;


    struct sockaddr_in server_addr;
    int sockfd;
    int PORT = atoi(argv[2]);

    if (argc != 2 ) {
        fprintf(stderr, "Usage: %s <url> <port>\n", argv[0]);
        return 1;
    }

    sockfd = get_socket(hostname, PORT);
    if (sockfd < 0) {
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
