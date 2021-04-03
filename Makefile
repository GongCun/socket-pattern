CC = gcc
CFLAGS = -g -O2 -Wall
LIBS = -lpthread
CLEANFILES = core core.* *.core *.o temp.* *.out
OBJS = server.o child.o error.o wrapsock.o wrapunix.o wrappthread.o takesocket.o givesocket.o

PROGS = tcpserver tcpserver2 tcpserver3 server
all: ${PROGS}

tcpserver: tcpserver.cpp
	g++ -std=c++14 -Wall $< -o $@

tcpserver2: tcpserver2.cpp
	g++ -std=c++14 -Wall $< -o $@

tcpserver3: tcpserver3.cpp
	g++ -std=c++14 -Wall $< -o $@ ${LIBS}

server:	${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f ${PROGS} ${CLEANFILES}
