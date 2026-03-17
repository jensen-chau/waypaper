#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "context.h"
#include "ipc.h"

int running = 1;
static pthread_t render_thread = 0;
static struct Context* ctx = NULL;

void* render_thread_func(void* arg) {
    run();
    return NULL;
}

void handle_set(char* path) {
    printf("Setting wallpaper: %s\n", path);

    // 初始化 context
    if (!ctx) {
        ctx = get_context(1920, 1080);
        if (!ctx) {
            printf("Failed to initialize context\n");
            return;
        }
    }

    // 加载壁纸
    int loaded = load_wallpaper(path);
    if (loaded == -1) {
        printf("Failed to load wallpaper: %s\n", path);
        return;
    }

    // 如果渲染线程还没运行，创建它
    if (render_thread == 0) {
        if (pthread_create(&render_thread, NULL, render_thread_func, NULL) != 0) {
            printf("Failed to create render thread\n");
            return;
        }
        printf("Render thread started\n");
    } else {
        // 如果渲染线程已运行，需要触发重新渲染
        // 这里可以添加逻辑来通知线程更新
    }
}

void handle_message(char* message, char* response, size_t response_size) {
    char* cmd = strtok(message, " ");

    if (cmd == NULL) {
        cmd = message;
    }


    if (strcmp(cmd, CMD_SET) == 0) {
        char* path = strtok(NULL, " ");
        if (path == NULL || strlen(path) == 0) {
            strncpy(response, "Missing path", response_size - 1);
            return;
        }

        handle_set(path);
        strncpy(response, "OK", response_size - 1);

    } else if (strcmp(cmd, CMD_SHUTDOWN) == 0) {
        // 停止渲染线程
        printf("Shutting down daemon...\n");
        running = 0;
        app_exit();

        // 等待渲染线程结束
        if (render_thread != 0) {
            pthread_join(render_thread, NULL);
            render_thread = 0;
            printf("Render thread stopped\n");
        }

        strncpy(response, "Shutting down", response_size - 1);

    } else if (strcmp(cmd, CMD_HELP) == 0) {
        strncpy(response, "help", response_size - 1);
    }

    else {
        strncpy(response, "Unknown command", response_size - 1);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[1024];
    char response[256];
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Daemon listening on %s...\n", SOCKET_PATH);

    while (running) {
        if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
            if (running) {
                perror("accept");
            }
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);

            memset(response, 0, sizeof(response));
            handle_message(buffer, response, sizeof(response));

            printf("Response: %s\n", response);
            write(client_fd, response, strlen(response));
        }

        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    printf("Daemon shutdown\n");

    return 0;
}
