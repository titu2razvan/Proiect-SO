#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "helper.h"
#include "commands.h"

int commandSelector(int argc, char **argv){
    char *cmd = getCommand(argc, argv);

    if(cmd == NULL) return 0;
    if(strcmp(cmd, "--add") == 0) return 1;
    if(strcmp(cmd, "--list") == 0) return 2;
    if(strcmp(cmd, "--view") == 0) return 3;
    if(strcmp(cmd, "--remove_report") == 0) return 4;
    if(strcmp(cmd, "--update_threshold") == 0) return 5;
    if(strcmp(cmd, "--filter") == 0) return 6;
    if(strcmp(cmd, "--remove_district") == 0) return 7;

    return 0;
}

void selectCommand(int argc, char **argv){
    int selected = commandSelector(argc, argv);

    switch(selected){
        case 1:
            add(argc, argv);
            break;
        case 2:
            list(argc, argv);
            break;
        case 3:
            view(argc, argv);
            break;
        case 4:
            remove_report(argc, argv);
            break;
        case 5:
            update_threshold(argc, argv);
            break;
        case 6:
            filter(argc, argv);
            break;
        case 7:
            remove_district(argc , argv);
            break;
        default:
            fprintf(stderr, "Unknown command.\n");
            exit(1);
    }
}

int main(int argc, char **argv){
    if(getRole(argc, argv) == NULL){
        fprintf(stderr, "Missing --role\n");
        return 1;
    }

    if(getUser(argc, argv) == NULL){
        fprintf(stderr, "Missing --user\n");
        return 1;
    }

    if(getCommand(argc, argv) == NULL){
        fprintf(stderr, "Missing command\n");
        return 1;
    }

    selectCommand(argc, argv);
    return 0;
}
