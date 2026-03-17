#include "ipc.h"
#include <stdio.h>

const char* CMD_SET = "set";
const char* CMD_SHUTDOWN = "shutdown";
const char* CMD_HELP = "help";

void help() {
    printf("Usage: waypaper [COMMAND]\n");
    printf("Commands:\n");
    printf("  set [PATH] - Set the wallpaper\n");
    printf("  shutdown - Shutdown the server\n");
    printf("  help - Show this help message\n");
    printf("\n");
}
