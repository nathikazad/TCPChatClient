#ifndef cclient_h
#define cclient_h

#include "util.h"
#define BUFFER_LENGTH 1000
class CClient
{
	public: 
		CClient(int socket);
		void handshake(char* handleName);
		int handshakeSuccess();
		void run();
	private:
		int _socket;
		char handleName[MAX_HANDLE_SIZE];
		int handleLength;
		uint32_t numberOfHandles;

		void sendCommand();
		int parsePacket();
		void forwardMessage(char *message);
		void bufferForwardMessage(char *command, int destHandleLength, int bufferSize, int start, int end);
		void broadcastMessage(char *message);
		void bufferBroadcastMessage(char *command, int bufferSize, int start, int end);
		void printMessage(char* message);
		void printBroadcastMessage(char* message);
		void printHandleDoesNotExist(char* message);
		void printNumberOfHandles(char* message);
        void printHandles(char* message);
};
#endif