#ifndef IPC_H
#define IPC_H

#define SOCKET_PATH "/tmp/waypaper.socket"

typedef enum IPC_MESSAGE {
    SET,
    SHUTDOWN
} IPC_MESSAGE;


typedef enum IPC_ERROR {
    IPC_SUCCESS = 0,
    IPC_SHUTDOWN,
    IPC_INVALID_MESSAGE,
} IPC_ERROR;

extern const char* CMD_SET;
extern const char* CMD_SHUTDOWN;
extern const char* CMD_HELP;

void help();

#endif
