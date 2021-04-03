/* include sockh */
#ifndef	__mysock_h
#define	__mysock_h
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
#include <pthread.h>

#ifndef	CMSG_LEN
#define	CMSG_LEN(size) (sizeof(struct cmsghdr) + (size))
#endif
#ifndef	CMSG_SPACE
#define	CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
#endif

#define SERV_PORT 9989
#define LISTENQ 5
#define MAXLINE 4096
typedef struct sockaddr SA;

void   err_dump(const char *, ...);
void   err_msg(const char *, ...);
void   err_quit(const char *, ...);
void   err_ret(const char *, ...);
void   err_sys(const char *, ...);

ssize_t	 Read(int, void *, size_t);
void	 Write(int, void *, size_t);
void	 Close(int);
void	 Dup2(int, int);
pid_t	 Fork(void);

int		 Accept(int, SA *, socklen_t *);
void	 Bind(int, const SA *, socklen_t);
void	 Connect(int, const SA *, socklen_t);
void	 Listen(int, int);
int		 Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
void	 Setsockopt(int, int, int, const void *, socklen_t);
int		 Socket(int, int, int);
void	 Socketpair(int, int, int, int *);

void Pthread_create(pthread_t *, const pthread_attr_t *,
                    void * (*)(void *), void *);
void Pthread_detach(pthread_t);

ssize_t givesocket(int, void *, size_t, int);
ssize_t takesocket(int, void *, size_t, int *);


#endif /* __mysock_h */
