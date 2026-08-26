#include <_stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// #include <openssl/ssl.h>
// #include <openssl/err.h>

#define BUFFER_SIZE 4096

typedef struct {
    char *protocol;
    char *hostname;
    char *route;
} url_t;

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

url_t parse_url(char *url) {
    url_t result = {NULL, NULL, NULL};
    if (!url) return result;

    // protocol
    char *protocol = strstr(url, ":");
    if (!protocol) return result;

    int protocol_len = protocol - url;
    result.protocol = (char *)malloc(protocol_len + 1);
    if (result.protocol) {
        strncpy(result.protocol, url, protocol_len);
        result.protocol[protocol_len] = '\0';
    }

    // hostname
    char *domain_start = protocol + 3;
    char *slash = strstr(domain_start, "/");

    int hostname_len;
    if (slash != NULL) { // route found
        hostname_len = slash - domain_start;
    } else { // no route
        hostname_len = strlen(domain_start);
    }

    result.hostname = (char *)malloc(hostname_len + 1);
    if (result.hostname) {
        strncpy(result.hostname, domain_start, hostname_len);
        result.hostname[hostname_len] = '\0';
    }

    // route
    if (slash && *slash != '\0') {
        int route_len = strlen(slash);
        result.route = (char *)malloc(route_len + 1);
        if (result.route) {
            strncpy(result.route, slash, route_len);
            result.route[route_len] = '\0';
        }
    }

    return result;
}

void free_url(url_t *url) {
    if (url->protocol) free(url->protocol);
    if (url->hostname) free(url->hostname);
    if (url->route) free(url->route);
}

// part of Tristan Hundley's Medium post studies
// executes GET / requests
int main(int argc, char *argv[]) {
    char *url = argv[1];
    url_t parsed = parse_url(url);

    printf("protocol: %s\n", parsed.protocol);
    printf("hostname: %s\n", parsed.hostname);
    printf("route: %s\n", parsed.route);

    if (strcmp(parsed.protocol, "https") == 0) {
        printf("HTTPS request\n");
        // TODO: handle https request with openssl
    } else if (strcmp(parsed.protocol, "http") == 0) {
        printf("HTTP request\n");
        // TODO: handle http request with sockets
    }

    free_url(&parsed);
    return 0;
}
