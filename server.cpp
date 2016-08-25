#include "server.h"

Server::Server(in_port_t portNumber)
{
	struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(portNumber);                 //let system choose the port

    /* bind the name (address) to a port */
    if (bind(_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
    {
		perror("bind call");
		exit(-1);
    }
    
    //get the port name and print it out
    if (getsockname(_socket, (struct sockaddr*)&local, &len) < 0)
    {
		perror("getsockname call");
		exit(-1);
    }

    printf("socket has port %d \n", ntohs(local.sin_port));

    if (listen(_socket, BACKLOG) < 0)
    {
      perror("listen call");
      exit(-1);
    }
    head = NULL;
    tail = NULL;
    maxClientSocket = _socket;
}

void Server::acceptClient()
{
	// printf("Accepting Client\n");
	int clientSocket = 0; 
	if ((clientSocket= accept(_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
    if(clientSocket > maxClientSocket)
    	maxClientSocket = clientSocket;
    if(head == NULL)
    {
		head = new Client(&fdSet, clientSocket, NULL);
		tail = head;
    }
	else
	{
		tail->next = new Client(&fdSet, clientSocket, tail);
		tail = tail->next;
	}

}

void Server::run()
{
	while(1)
	{
		FD_ZERO(&fdSet);
		FD_SET(_socket, &fdSet);
		setClientFDs();
		if (select(maxClientSocket+1, &fdSet, NULL, NULL, NULL) < 0){
			perror("server select");
			exit(-1);
		}
		if (FD_ISSET(_socket, &fdSet)) 
			acceptClient();	
		checkClientFDs();
	}
}

void Server::setClientFDs()
{
	Client *iter = head;
	while(iter != NULL)
	{
		iter->setFD();
		iter = iter->next;
	}
}

void Server::checkClientFDs()
{
	Client *iter = head;
	while(iter != NULL)
	{
		if(iter->checkFD())
		{
			printf("Closing client\n");
			if((int)iter == (int)head)
				head = head->next;
			else
				iter->prev->next = iter->next;
			if((int)iter == (int)tail)
				tail = tail->prev;
			else
				iter->next->prev = iter->prev;
			Client* temp = iter;
			iter = iter->next;	
			delete temp;
		}
		else
			iter = iter->next;
	}
}

Server::~Server()
{
	close(_socket);
}