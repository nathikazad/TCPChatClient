#include "client.h"


Client::Client(fd_set* fdSet, int socket, Client *prev)
{
	this->_socket = socket;
	this->fdSet = fdSet;
	this->prev = prev;
	state = 0;
	next = NULL;
}

Client::~Client()
{
	close(_socket);
	delete[] handleName;
}

void Client::setFD()
{
	FD_SET(_socket, fdSet);
}

int Client::socket()
{
	return _socket;
}
int Client::checkFD()
{
	if (FD_ISSET(_socket, fdSet)) 
	{
  		int packet_len= 0;     //length of the received message
  		char packet[MAX_PACKET_SIZE];
  		for(int i=0;i<MAX_PACKET_SIZE;i++)
        	packet[i]=0;
		if ((packet_len = recv(_socket, packet, 2, 0)) < 0)
		{
	       perror("recv call");
	       exit(-1);
	    }
	    if(packet_len==0)
	    	return 1;
	    int packetSize = ntohs((packet[0] << 8) + packet[1]);
	    if ((packet_len = recv(_socket, packet+2, packetSize-2, 0)) < 0)
		{
	       perror("recv call");
	       exit(-1);
	    }
	    
	    if(processPacket(packet)) 
	    	return 1;
	}
	return 0;
}

int Client::processPacket(char* packet)
{
	if(state == 0)
	{
		// printf("Handshake check\n");
		handleLength = packet[3];
		handleName = new char[handleLength];
		int i;
		for(i=0; i < handleLength; i++)
			handleName[i] = packet[4+i];
		handleName[i]=0;
		int found = (findClientWithHandle(handleName)!=NULL)?1:0;
		respondToHandshakeRequest(found);
		state = 1;
	}
	else
	{
		if(packet[2]==5)//Message
			forwardMessage(packet);
		else if(packet[2]==4)//Broadcast
			broadcastMessage(packet);
		else if(packet[2]==10)//List
			listHandles();
		else if(packet[2]==8)//Exit
		{
			sendOnlyFlag(_socket, 9);
			return 1;
		}
	}
	return 0;
}

Client* Client::findClientWithHandle(char* handleName)
{
	Client* found = NULL;
	if(strcmp(this->handleName, handleName)==0)
	{
			return NULL;
	}
	Client* iter = this->prev;
	while(iter != NULL)
	{
		if(strcmp(iter->handleName, handleName)==0)
			found = iter;
		iter = iter->prev;
	}
	iter = this->next;
	while(iter != NULL)
	{
		if(strcmp(iter->handleName, handleName)==0)
			found = iter;
		iter = iter->next;
	}
	return found;
}
int Client::respondToHandshakeRequest(int found){
	// (found)? printf("handle conflict\n") : printf("no handle conflict\n");
	// printf("Responding to client\n");
	sendFlag(found==1 ? 3 : 2 );
	return found;
}

void Client::forwardMessage(char* payload)
{
	printf("Forwarding Message\n");
	int destHandleLength = payload[3];
	char destHandle[destHandleLength];
	for(int i=0;i<destHandleLength;i++)
		destHandle[i]=payload[4+i];
	destHandle[destHandleLength]=0;
	Client* destClient = findClientWithHandle(destHandle);
	if(destClient!=NULL)
		destClient->sendMessage(payload);
	else if(strcmp(destHandle, handleName)==0)
		this->sendMessage(payload);
	else
	{
		int responseSize = 4+destHandleLength;
		char response[responseSize];
		response[0] = (htons(responseSize) & 0xFF00) >> 8;
  		response[1] = htons(responseSize) & 0xFF;
  		response[2] = 7;
  		response[3] = destHandleLength;
  		for(int i=0;i<destHandleLength;i++)
  			response[4+i] = destHandle[i];
  		this->sendMessage(response);
	}	
}

void Client::broadcastMessage(char* payload)
{
	Client* iter = this->prev;
	while(iter != NULL)
	{
		iter->sendMessage(payload);
		iter = iter->prev;
	}
	iter = this->next;
	while(iter != NULL)
	{
		iter->sendMessage(payload);
		iter = iter->next;
	}
}

void Client::listHandles()
{
	printf("Listing handles for %s\n",handleName);
	uint32_t numberOfHandles = 1;
	int totalHandleLength = this->handleLength;
	Client* iter = this->prev;
	Client* head =  this;
	while(iter != NULL)
	{
		numberOfHandles++;
		totalHandleLength+=iter->handleLength;
		if(iter->prev == NULL)
			head = iter;
		iter = iter->prev;
	}
	iter = this->next;
	while(iter != NULL)
	{
		totalHandleLength+=iter->handleLength;
		numberOfHandles++;
		iter = iter->next;
	}
	int responseSize = 7;
	char response[responseSize];
	response[0] = (htons(responseSize) & 0xFF00) >> 8;
	response[1] = htons(responseSize) & 0xFF;
	response[2] = 11;
	numberOfHandles = htonl(numberOfHandles);
	response[3] = (numberOfHandles & 0xFF000000) >> 24;
	response[4] = (numberOfHandles & 0xFF0000) >> 16;
	response[5] = (numberOfHandles & 0xFF00) >> 8;
	response[6] = (numberOfHandles & 0xFF) >> 0;
	this->sendMessage(response);

	numberOfHandles = ntohl(numberOfHandles);
	int listResponseSize = 3+numberOfHandles+totalHandleLength;
	char listResponse[listResponseSize];
	listResponse[0] = 0;
	listResponse[1] = 0;
	listResponse[2] = 12;
	int i=3;
	while(head!=NULL)
	{
		listResponse[i]=head->handleLength;
		i++;
		for(int j=0; j<head->handleLength;j++)
		{
			listResponse[i]=head->handleName[j];
			i++;
		}
		head = head->next;
	}
	int chars;
	if((chars = send(_socket, listResponse, listResponseSize, 0)) < 0)
    {
        perror("send call");
        exit(-1);
    }
    printf("Handles sent %d\n", chars);
}

void Client::sendMessage(char* message)
{
	int packetLength = htons((message[0] << 8) + message[1]);
	printf("Packet Length %d\n",packetLength);
	if(send(_socket, message, packetLength, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}
 
 void Client::sendFlag(int flag)
 {
 	sendOnlyFlag(_socket, flag);
 }