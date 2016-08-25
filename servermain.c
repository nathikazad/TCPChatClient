
#include "util.h"
#include "server.h"


int tcp_recv_setup();
int tcp_listen(int server_socket, int back_log);

int main(int argc, char *argv[])
{
  int portNumber = 0;
  if(argc == 2)
    portNumber = atoi(argv[1]);
  else if(argc != 1)
  {
    printf("Usage server OPTIONAL:<portnumber>");
    return -1;
  }
  Server server(portNumber);
  server.run(); 
}


