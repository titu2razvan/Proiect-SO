#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#define PID_FILE ".monitor_pid"

static volatile sig_atomic_t running = 1;

static void on_sigusr1(int sig){
    (void)sig;
    static const char msg[] = "Monitor: new report was added.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

static void on_sigint(int sig){
    (void)sig;
    running = 0;
}

static void write_pid_file(void){
    char buf[32];
    int fd;
    int len;

    fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open .monitor_pid");
        exit(1);
    }
    len = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if(write(fd, buf, len) != len){
        perror("write .monitor_pid");
        close(fd);
        exit(1);
    }
    close(fd);
}

static void remove_pid_file(void){
    unlink(PID_FILE);
}

int main(void){
    struct sigaction sa;

    write_pid_file();

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sa.sa_handler = on_sigusr1;
    if(sigaction(SIGUSR1, &sa, NULL) == -1){
        perror("sigaction SIGUSR1");
        remove_pid_file();
        return 1;
    }

    sa.sa_handler = on_sigint;
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction SIGINT");
        remove_pid_file();
        return 1;
    }

    printf("Monitor started. PID: %d\n", (int)getpid());
    fflush(stdout);

    while(running){
        pause();
    }

    printf("Monitor shutting down.\n");
    remove_pid_file();
    return 0;
}
