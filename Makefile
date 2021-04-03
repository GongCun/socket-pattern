CC = gcc
CFLAGS = -g -O2 -Wall
LIBS = -lpthread
CLEANFILES = core core.* *.core *.o temp.* *.out
OBJS = server.o child.o error.o wrapsock.o wrapunix.o wrappthread.o takesocket.o givesocket.o

PROGS = server
all: ${PROGS}

server:	${OBJS}
		${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
