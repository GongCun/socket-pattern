#include "sock.h"

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;
    int child_make(int);

    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    const int on = 1;
    // SO_REUSEADDR allows a listening server to start and bind its well-known
    // port, even if previously established connections exist that use this port
    // as their local port.
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    int sockfd = child_make(listenfd);
    int maxfd = sockfd > listenfd ? sockfd : listenfd;

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    FD_SET(sockfd, &allset);

    int nsel, connfd, ret;

    for ( ; ; ) {
        rset = allset;
        nsel = Select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            connfd = Accept(listenfd, (SA *)NULL, NULL);
            
            ret = givesocket(sockfd, &connfd, sizeof(connfd), connfd);
            if (ret != sizeof(connfd))
                err_quit("write_fd returned unexpectedly");

            printf("give connfd %d\n", connfd);

            if (--nsel == 0)
                continue; /* all done with select() results */
        }

        if (FD_ISSET(sockfd, &rset)) {
            Read(sockfd, &connfd, sizeof(connfd));
            printf("parent will close %d\n", connfd);
            Close(connfd);
        }
    }
}
