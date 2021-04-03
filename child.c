#include	"sock.h"

int
child_make(int listenfd)
{
    int		sockfd[2];
    pid_t	pid;
    void	child_main();

    Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

    if ((pid = Fork()) > 0) {
        Close(sockfd[1]);
        return sockfd[0]; /* parent */
    }

    Dup2(sockfd[1], STDERR_FILENO); /* child's stream pipe to parent */
    Close(sockfd[0]);
    Close(sockfd[1]);
    Close(listenfd); /* child does not need this open */
    child_main();	/* never returns */
}

void
child_main()
{
    int			  fd;
    int				connfd;
    pthread_t tid;
    void			*doit(void *);

    printf("child %ld starting\n", (long) getpid());

    for ( ; ; ) {
        if (takesocket(STDERR_FILENO, &fd, sizeof(fd), &connfd) != sizeof(fd))
            err_quit("read_fd returned unexpectedly");

        if (connfd < 0)
            err_quit("no descriptor from read_fd");

        printf("parent connfd %d, received connfd %d\n", fd, connfd);

        Pthread_create(&tid, NULL, doit, (void *)connfd);

        Write(STDERR_FILENO, &fd, sizeof(fd)); /* tell parent we're ready again */
    }
}

void *doit(void *arg)
{
    int len;
    char buf[MAXLINE];
    int connfd = (int)arg;
    Pthread_detach(pthread_self());

  again:
    while ((len = read(connfd, buf, MAXLINE)) > 0) {
        int ret;
        char *ptr = buf;
        while (len != 0 && (ret = write(connfd, ptr, len)) != 0) {
            if (ret == -1) {
                if (errno == EINTR)
                    continue;
                printf("write: %s\n", strerror(errno));
                break;
            }
            len -= ret;
            ptr += ret;
        }
    }

    if (len < 0 && errno == EINTR)
        goto again;
    else if (len < 0) {
        printf("read: %s\n", strerror(errno));
    }

    Close(connfd);
}
