#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;
using SA = struct sockaddr;
const int SERV_PORT = 9989;
const int LISTENQ = 5;
const int MAXLINE = 4096;

void err_exit(const char *s) {
    perror(s);
    exit(-1);
}

void sig_chld(int signo) {
    while (waitpid(-1, (int *)NULL, WNOHANG) > 0)
        ;
    return;
}

void doit(int sockfd) {
    int len;
    char buf[MAXLINE];

  again:
    while ((len = read(sockfd, buf, MAXLINE)) > 0) {
        int ret;
        char *ptr = buf;
        while (len != 0 && (ret = write(sockfd, ptr, len)) != 0) {
            if (ret == -1) {
                if (errno == EINTR)
                    continue;
                err_exit("write");
            }
            len -= ret;
            ptr += ret;
        }
    }

    if (len < 0 && errno == EINTR)
        goto again;
    else if (len < 0) {
        err_exit("read");
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        err_exit("socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    const int on = 1;
    // SO_REUSEADDR allows a listening server to start and bind its well-known
    // port, even if previously established connections exist that use this port
    // as their local port.
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        err_exit("setsockopt");
    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
        err_exit("bind");

    if (listen(listenfd, LISTENQ) < 0)
        err_exit("listen");

    if (signal(SIGCHLD, sig_chld) == SIG_ERR)
        err_exit("signal");

    for ( ; ; ) {
        int connfd = accept(listenfd, (SA *)NULL, NULL);
        if (connfd < 0) {
            if (errno == EINTR)
                continue; // back to fork()
            err_exit("accept");
        }

        pid_t pid = fork();
        if (pid < 0)
            err_exit("fork");
        else if (pid == 0) {       // child process
            if (close(listenfd) < 0)
                err_exit("close"); // close listening socket
            doit(connfd);          // process request
            exit(0);               // child process exit
        }
        if (close(connfd) < 0)     // parent closes connected socket
            err_exit("close");
    }
}
