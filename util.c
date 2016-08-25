#include "util.h"
void sendOnlyFlag(int _socket, int flag)
{
	char packet[3];
  uint16_t size = 3;
  packet[0] = (htons(size) & 0xFF00) >> 8;
  packet[1] = htons(size) & 0xFF;
  packet[2] = flag;
  int sent =  send(_socket, packet, 3, 0);
  if(sent < 0)
  {
     perror("send call");
     exit(-1);
  }
}