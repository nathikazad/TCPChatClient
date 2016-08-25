#ifndef client_h
#define client_h

#include "util.h"

class Client
{
	public: 
		Client(fd_set* fdSet, int socket, Client *prev);
		~Client();
		void setFD();
		int checkFD();
		int socket();
		void sendMessage(char* message);
		void sendFlag(int flag);
		Client *next;
		Client *prev;
		char* handleName;
		int handleLength;
	private:
		int _socket;
		fd_set* fdSet;
		int state; //handshake->0, connected->1
		int processPacket(char* packet);
		int respondToHandshakeRequest(int found);
		Client* findClientWithHandle(char* handleName);
		void forwardMessage(char* payload);
		void broadcastMessage(char* payload);
		void listHandles();
};
#endif