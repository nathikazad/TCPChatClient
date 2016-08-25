#ifndef server_h
#define server_h
#include "util.h"
#include "client.h"

class Server 
{
	public: 
		Server(in_port_t portNumber);
		void run();
		~Server();

	private:
		int _socket;
		fd_set fdSet;
		int maxClientSocket;
		Client* head;
		Client* tail;

		void setClientFDs();
		void acceptClient();
		void checkClientFDs();
		//temp
		
};
#endif