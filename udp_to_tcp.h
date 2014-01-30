#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */ 
#include <stdlib.h>

struct tcp_header
{
  int seqNum;
  int ackNum;
  char permissions1;
  char permissions2;
  short windowSize;
  short checksum;
  short urgentPointer;
};

class tcp
{
  string ip;
  int port;
  int socket;
  sockaddr_in remoteAddress;
  bool connectionEstablished;

  int establish();
  void listen();
  bool kill();
  int send(string &data);
  int receive(string &data);
  bool sendPacket(string &data);
  bool receivePacket(string &data);
};
