#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct {
    char *protocol;
    char *hostname;
    char *route;
} url_t;

// this function implements part of Tristan Hundley's Medium post
int socket_prepare(url_t *url, char *port) {
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof(hints)); // reset hints
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp

    if (getaddrinfo(url->hostname, port, &hints, &res) != 0) {
        printf("Error: could not resolve hostname DNS\n");
        return 1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, 0);
    if (sockfd == -1) {
        printf("Error: could not create socket\n");
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("Error: could not connect to server\n");
        freeaddrinfo(res);
        close(sockfd);
        return 1;
    }

    freeaddrinfo(res);
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

char *dynamic_download(int sockfd, SSL *ssl) {
    int initial_capacity = 4096;
    int read = 0;
    char *buffer = (char *)malloc(initial_capacity);

    int received;

    while (1) {
        if (ssl) {
            received = SSL_read(ssl, buffer + read, initial_capacity - read);
        } else {
            received = recv(sockfd, buffer + read, initial_capacity - read, 0);
        }

        if (received <= 0) break;

        read += received;
        if (read == initial_capacity) {
            initial_capacity *= 2;

            char *new_buffer = (char *)realloc(buffer, initial_capacity);
            if (!new_buffer) {
                printf("dynamic memory buffer realloc error");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
    }

    char *result = (char *)malloc(read + 1);
    if (result) {
        strncpy(result, buffer, read);
        result[read] = '\0';
        free(buffer);
        return result;
    }

    free(buffer);
    return NULL;
}

int is_valid_method(char *verb) {
    char *methods[] = {"GET", "POST", "PUT", "PATCH", "DELETE", NULL};
    for (int i = 0; methods[i] != NULL; i++) {
        if (strcmp(verb, methods[i]) == 0) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        perror("Usage: ./revolver <url> <verb> <(OPTIONAL) payload between quotes OR @ + json file name (e.g. @payload.json (attach file in this same directory))>");
        return 1;
    }

    char *url = argv[1];
    if (!url) perror("Usage: ./revolver <url> <verb> <?payload between quotes | @ + json file name>");
    url_t parsed = parse_url(url);

    char *verb = argv[2];
    if (!verb) perror("Usage: ./revolver <url> <verb> <(OPTIONAL) payload between quotes OR @ + json file name (e.g. @payload.json (attach file in this same directory))>");
    if (!is_valid_method(verb)) {
        perror("Invalid method (supported methods: GET, POST, PUT, PATCH, DELETE)");
        return 1;
    }

    char *payload = NULL;
    if (argc == 4) {
        if (argv[3][0] == '@') {
            FILE *payload_file = fopen(argv[3] + 1, "rb");
            if (!payload_file) {
                perror("Failed to open json file");
                return 1;
            }

            fseek(payload_file, 0, SEEK_END);
            unsigned long payload_size = ftell(payload_file);
            rewind(payload_file);

            char *payload_buffer = (char *)malloc(payload_size + 1);
            if (!payload_buffer) {
                perror("Failed to allocate memory for payload buffer");
                fclose(payload_file);
                return 1;
            }

            size_t bytes_read = fread(payload_buffer, 1, payload_size, payload_file);
            if (bytes_read != payload_size) {
                perror("Failed to read payload from file");
                fclose(payload_file);
                free(payload_buffer);
                return 1;
            }

            payload_buffer[payload_size] = '\0';
            payload = payload_buffer;

            fclose(payload_file);
        } else {
            payload = argv[3];
        }
    }

    int is_https = (strcmp(parsed.protocol, "https") == 0);
    char *port = is_https ? "443" : "80";

    printf("protocol: %s, hostname: %s, route: %s, verb: %s\n", parsed.protocol, parsed.hostname, parsed.route, verb);

    int sockfd = socket_prepare(&parsed, port);
    if (sockfd == -1) {
        free_url(&parsed);
        return 1;
    }
    // connected

    int payload_bytes = payload ? strlen(payload) : 0;
    char request_headers[1024];
    char *route = parsed.route ? parsed.route : "/";
    snprintf(request_headers, sizeof(request_headers),
        "%s %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n"
        "\r\n",
        verb, route, parsed.hostname, payload_bytes);

    if (is_https == 1) {
        // https
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
        SSL *ssl = SSL_new(ctx);

        SSL_set_fd(ssl, sockfd);

        if (SSL_connect(ssl) <= 0) {
            printf("Error: could not handshake (HTTPS)\n");
        } else {
            SSL_write(ssl, request_headers, strlen(request_headers));
            if (payload) SSL_write(ssl, payload, payload_bytes);
            printf("The HTTPS server responded with...\n");
            char *response = dynamic_download(sockfd, ssl);
            printf("\x1b[32m%s\x1b[0m\n", response);
            free(response);
        }
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    } else {
        // http
        send(sockfd, request_headers, strlen(request_headers), 0);
        if (payload) send(sockfd, payload, payload_bytes, 0);
        printf("The HTTP server responded with...\n");
        char *response = dynamic_download(sockfd, NULL);
        printf("\x1b[32m%s\x1b[0m\n", response);
        free(response);
    }

    if (argc == 4 && argv[3][0] == '@') free(payload);
    free_url(&parsed);
    close(sockfd);
    return 0;
}
