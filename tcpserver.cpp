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

using namespace std;
using SA = struct sockaddr;
const int SERV_PORT = 9989;
const int LISTENQ = 5;
const int MAXLINE = 4096;


int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;

    auto err_exit = [](const char *s) {
        perror(s);
        exit(-1);
    };
    
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        err_exit("socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
        err_exit("bind");

    if (listen(listenfd, LISTENQ) < 0)
        err_exit("listen");

    char buf[MAXLINE];
    int maxfd = listenfd;
    int maxi = -1;
    vector<int> client(FD_SETSIZE, -1);
    fd_set allset;

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; ) {
        fd_set rset = allset;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0)
            err_exit("select");

        if (FD_ISSET(listenfd, &rset)) {
            int connfd = accept(listenfd, (SA *)NULL, NULL);
            if (connfd < 0)
                err_exit("accept");

            int i = 0;
            for (; i < client.size(); ++i) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }
            if (i >= client.size()) {
                cerr << "too many client" << endl;
                exit(-1);
            }

            FD_SET(connfd, &allset);
            maxfd = max(maxfd, connfd);
            maxi = max(maxi, i);

            if (--nready <= 0)
                continue;
        }

        int sockfd;
        for (int i = 0; i <= maxi; ++i) {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset)) {
                int len = read(sockfd, buf, MAXLINE);
                if (len < 0) {
                    err_exit("read");
                } else if (len == 0) {
                    if (close(sockfd) < 0)
                        err_exit("close");
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
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

                if (--nready <= 0)
                    break;
            }
        }
    }

}
