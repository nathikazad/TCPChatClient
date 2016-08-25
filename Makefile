CC= g++
CFLAGS= -g

# The  -lsocket -lnsl are needed for the sockets.
# The -L/usr/ucblib -lucb gives location for the Berkeley library needed for
# the bcopy, bzero, and bcmp.  The -R/usr/ucblib tells where to load
# the runtime library.

# The next line is needed on Sun boxes (so uncomment it if your on a
# sun box)
# LIBS =  -lsocket -lnsl

# For Linux/Mac boxes uncomment the next line - the socket and nsl
# libraries are already in the link path.

all:  cclient server

cclient: clientmain.c cclient.cpp util.h util.c testing.o
	$(CC) $(CFLAGS) -o cclient -Wall util.c clientmain.c cclient.cpp testing.o -I.

server: servermain.c server.cpp client.cpp util.h util.c testing.o
	$(CC) $(CFLAGS) -o server -Wall util.c servermain.c server.cpp client.cpp testing.o -I.

testing.o: testing.c testing.h
	   $(CC) $(CFLAGS) -c testing.c

clean:
	rm -f server cclient testing.o