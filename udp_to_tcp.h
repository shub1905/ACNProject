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
#include <cstdlib>

#define LOCAL_SERVER_PORT 1500
#define REMOTE_SERVER_PORT 1500
#define MAX_MSG 100
#define TIMEOUT_VAL 2

using namespace std;

typedef struct tcp_header
{
  int length;
  int seqNum;
  int ackNum;
  char permissions1;
  char permissions2;
  short windowSize;
  short checksum;
  short urgentPointer;
} tcp_header;

class tcp
{
  string ip;
  int port;
  int sock;
  struct sockaddr_in remoteAddress;
  bool connectionEstablished = false;
  int seqnumber; //to set
  int datatosend;
  int sendack; //to set
  int recvack;

  int establish();
  void listen();
  bool kill();
  int send(string &data);
  int receive(string &data);
  bool sendPacket(string &data);
  bool receivePacket(string &data);
};
