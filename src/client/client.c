#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipc.h"


int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_un addr;
    char buffer[1024];

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    char *msg = malloc(1024);
    if (!msg) {
        perror("malloc");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    memset(msg, 0, 1024);

    int len = 0;

    for(int i=1; i< argc; i++) {
        if (i > 1) {
            msg[len++] = ' ';
        }
        int arg_len = strlen(argv[i]);
        if (len + arg_len < 1023) {
            strcpy(msg + len, argv[i]);
            len += arg_len;
        }
    }

    msg[len] = '\0';

    if (write(sock_fd, msg, strlen(msg)) == -1) {
        perror("write");
        free(msg);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    free(msg);

    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, CMD_HELP) == 0) {
            help();
        }
        printf("Server replied: %s\n", buffer);
    }

    close(sock_fd);
    return 0;
}
