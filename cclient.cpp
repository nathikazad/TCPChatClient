#include "cclient.h"


CClient::CClient(int socket)
{
	this->_socket = socket;
}

void CClient::handshake(char* handleName)
{
    handleLength = strlen(handleName);
    int packetSize = 4+handleLength;
    char packet[packetSize];

    //NETWORK ORDER
    packet[0] = (htons(packetSize) & 0xFF00) >> 8;
    packet[1] = htons(packetSize) & 0xFF;//0xFF & bufsize;
    packet[2] = 1;
    packet[3] = handleLength;
    for(int i=0;handleName[i]!=0;i++)
    {
        packet[4+i] = handleName[i];
        this->handleName[i] = handleName[i];
    }
    this->handleName[handleLength] = 0;
    if(send(_socket, packet, packetSize, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

int CClient::handshakeSuccess()
{
    int buffer_size = 10;  
  	int message_len = 0;     
	char buf[buffer_size];
	if ((message_len = recv(_socket, buf, buffer_size, 0)) < 0)
    {
       perror("recv call");
       exit(-1);
    }
	buf[message_len]=0;
	if(buf[2]==2)
		return 1;
    else
    {
	   printf("Handle already in use: %s\n", handleName);
	   return 0;
    }

}

void CClient::run()
{
	fd_set fdSet;
    printf("$S: ");
    fflush(stdout);
    while(1)
    {
        FD_ZERO(&fdSet);
        FD_SET(_socket, &fdSet);
        FD_SET(STDIN_FILENO, &fdSet);
        if (select(_socket+1, &fdSet, NULL, NULL, NULL) < 0){
            perror("client select");
            exit(-1);
        }
            
        if (FD_ISSET(_socket, &fdSet))
        {
            if(parsePacket())
                return;
            printf("$S: ");
            fflush(stdout);
        }
        if (FD_ISSET(STDIN_FILENO, &fdSet))
        {
            sendCommand(); 
            printf("$S: ");
            fflush(stdout);
        }
    }
}

void CClient::sendCommand()
{
    int msg_len = 0;
    char message[4096];
    while ((message[msg_len] = getchar()) != '\n' && msg_len < 4096)
        msg_len++;
    message[msg_len] = '\0';
    if(msg_len==0)
        return;
    if(message[1]=='M' || message[1]=='m')
        forwardMessage(message);
    else if(message[1]=='B' || message[1]=='b')
        broadcastMessage(message);
    else if(message[1]=='E' || message[1]=='e')
        sendOnlyFlag(_socket, 8);
    else if(message[1]=='L' || message[1]=='l')
        sendOnlyFlag(_socket, 10);
    else
        printf("Invalid command\n");  


}

void CClient::forwardMessage(char *command)
{
    int destHandleLength = 0;
    while(command[3+destHandleLength]!=' ' && command[3+destHandleLength]!=0)
        destHandleLength++;
    int messageLength = 0;
    if(command[3+destHandleLength]==' ')
        while(command[4+destHandleLength+messageLength]!=0)
            messageLength++;
    int mp=0;
    for(mp=0; mp<messageLength/BUFFER_LENGTH;mp++)
        bufferForwardMessage(command, destHandleLength,BUFFER_LENGTH, mp*BUFFER_LENGTH, (mp+1)*BUFFER_LENGTH);
     bufferForwardMessage(command, destHandleLength,messageLength%BUFFER_LENGTH, mp*BUFFER_LENGTH, messageLength);
}

void CClient::bufferForwardMessage(char *command, int destHandleLength,int bufferSize, int start, int end)
{
    int i;
    int packetLength = 3 + 1 + destHandleLength + 1 + handleLength + bufferSize;
    char packet[packetLength];
    packet[0] = (htons(packetLength) & 0xFF00) >> 8;
    packet[1] = htons(packetLength) & 0xFF;
    packet[2] = 5;
    packet[3] = destHandleLength;
    for(i=0; i<destHandleLength; i++)
        packet[4+i] = command[3+i];
    packet[4+i] = handleLength;
    for(i=0; i<destHandleLength; i++)
        packet[4+destHandleLength+1+i] = handleName[i];
    int j=0;
    for(i=start; i<end; i++)
    {
        packet[5+destHandleLength+handleLength+j] = command[4+destHandleLength+i];
        j++;
    }
    packet[packetLength]=0;
    if(send(_socket, packet, packetLength, 0) < 0)
    {
        perror("send call");
        exit(-1);
    } 
}

void CClient::broadcastMessage(char *command)
{
    int messageLength = 0;
    if(command[2]==' ')
        while(command[3+messageLength]!=0)
            messageLength++;

    int mp=0;
    for(mp=0; mp<messageLength/BUFFER_LENGTH;mp++)
        bufferBroadcastMessage(command, BUFFER_LENGTH, mp*BUFFER_LENGTH, (mp+1)*BUFFER_LENGTH);
     bufferBroadcastMessage(command, messageLength%BUFFER_LENGTH, mp*BUFFER_LENGTH, messageLength);

    
}

void CClient::bufferBroadcastMessage(char *command, int bufferSize, int start, int end)
{
    int packetLength = 3 + 1 + handleLength + bufferSize;
    char packet[packetLength];
    packet[0] = (htons(packetLength) & 0xFF00) >> 8;
    packet[1] = htons(packetLength) & 0xFF;
    packet[2] = 4;
    packet[3] = handleLength;
    for(int i=0; i<handleLength; i++)
        packet[4+i] = handleName[i];
    int j=0;
    for(int i=start; i<end; i++)
    {
        packet[4+handleLength+j] = command[3+i];
        j++;
    }
    if(send(_socket, packet, packetLength, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

int CClient::parsePacket()
{
    int buffer_size = MAX_PACKET_SIZE;  
    int message_len = 0;     
    char buf[buffer_size];
    for(int i=0;i<buffer_size;i++)
        buf[i]=0;
    if ((message_len = recv(_socket, buf, 2, 0)) < 0)
    {
       perror("recv call");
       exit(-1);
    }
    if(message_len==0)
    {
        printf("Server Terminated\n");
        return 1;
    }
    int packetSize = ntohs((buf[0] << 8) + buf[1]);
    if ((message_len = recv(_socket, buf+2, packetSize-2, 0)) < 0)
    {
       perror("recv call");
       exit(-1);
    }

    if(buf[2] == 5) //message
        printMessage(buf);
    else if(buf[2] == 4)
        printBroadcastMessage(buf);
    else if(buf[2] == 7) //destination handle does not exist
        printHandleDoesNotExist(buf);
    else if(buf[2] == 11) //number of handles
    {
        printNumberOfHandles(buf);
        int i;
        for(i=7;i<=MAX_PACKET_SIZE;i++)
            if(buf[i]==12)
                break;
        if(i < MAX_PACKET_SIZE)
            printHandles(buf+i-2);

    }
    else if(buf[2] == 12) //list of handles
        printHandles(buf);
    else if(buf[2] == 9) //exit
        return 1;

    return 0;
}

void CClient::printMessage(char* message)
{
    int destHandleLength = message[3];
    int srcHandleLength = message[3+1+destHandleLength];
    printf("\n");
    for(int i=0;i<srcHandleLength;i++)
     printf("%c", message[3+1+destHandleLength+1+i]);
    printf(": ");
    for(int i=0;message[3+1+destHandleLength+1+srcHandleLength+i]!=0;i++)
     printf("%c", message[3+1+destHandleLength+1+srcHandleLength+i]);
    printf("\n");
}
void CClient::printBroadcastMessage(char* message)
{
    int srcHandleLength = message[3];
    printf("\n");
    for(int i=0;i<srcHandleLength;i++)
     printf("%c", message[3+1+i]);
    printf(": ");
    for(int i=0;message[3+1+srcHandleLength+i]!=0;i++)
     printf("%c", message[3+1+srcHandleLength+i]);
    printf("\n");
}

void CClient::printHandleDoesNotExist(char* message)
{
    printf("Handle does not exist\n");
}

void CClient::printNumberOfHandles(char* message)
{
    numberOfHandles = message[3] << 24;
    numberOfHandles += message[4] << 16;
    numberOfHandles += message[5] << 8;
    numberOfHandles += message[6] ;
    numberOfHandles = ntohl(numberOfHandles);
    printf("Number of clients: %d\n",numberOfHandles);
}

void CClient::printHandles(char* message)
{
    int indexOfHandleLength = 3;
    for(uint32_t i=0; i<numberOfHandles;i++)
    {
        int handleLength = message[indexOfHandleLength];
        printf("\t");
        for(int j=indexOfHandleLength+1;j<=(indexOfHandleLength+handleLength);j++)
            printf("%c",message[j]);
        printf("\n");
        indexOfHandleLength += handleLength+1;
    }
    numberOfHandles = 0;
}
