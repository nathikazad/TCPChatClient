#include "util.h"
#include "cclient.h"

int tcp_send_setup(char *host_name, char *port);


int main(int argc, char * argv[])
{
    if(argc!= 4)
    {
        printf("usage: %s <handle> <host-name> <port-number> \n", argv[0]);
        exit(1);
    }
    int socket_num = tcp_send_setup(argv[2], argv[3]);
    CClient client(socket_num);
    client.handshake(argv[1]);
    if(client.handshakeSuccess())
        client.run();
}



int tcp_send_setup(char *host_name, char *port)
{
    int socket_num;
    struct sockaddr_in remote;       // socket address for remote side
    struct hostent *hp;              // address of remote host

    // create the socket
    if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror("socket call");
	    exit(-1);
	}
    

    // designate the addressing family
    remote.sin_family= AF_INET;

    // get the address of the remote host and store
    if ((hp = gethostbyname(host_name)) == NULL)
	{
	  printf("Error getting hostname: %s\n", host_name);
	  exit(-1);
	}
    
    memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

    // get the port used on the remote side and store
    remote.sin_port= htons(atoi(port));

    if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
    {
	perror("connect call");
	exit(-1);
    }

    return socket_num;
}