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
#include <pthread.h>

#define LOCAL_SERVER_PORT 1500
#define REMOTE_SERVER_PORT 1500
#define MAX_MSG 100
#define TIMEOUT_VAL 2
#define PACKETSIZE 1500
#define CWSIZE 100
#define TIMEOUT 5
#define BUF_SIZE_OS 1000

using namespace std;

long long min(long long, long long);

typedef struct thread_args
{
  void *object_pointer;
  int seqNum;
} thread_args;

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

class tcp {
  public:
    string ip;
    int port;
    int sock;
    struct sockaddr_in remoteAddress;
    bool connectionEstablished;
    int seqnumber; //to set
    int seqnumberRemote;
    int datatosend;
    int sendack; //to set
    int recvack;
    int numacks;
    pthread_mutex_t acklock;
    pthread_mutex_t pktloss;
    pthread_mutex_t timeoutlock;
    bool packetTimeout;
    char dataBuffer[BUF_SIZE_OS];
    bool bitmapReceive[BUF_SIZE_OS];
    int head;
    int tail;
    int remoteBaseSeqNumber;

    tcp();
    int establish();
    void listen();
    bool kill();
    int send(string &data);
    int receive(string &data);
    void receiveLoop();
    bool receivePacket(char *data);
    bool sendPacket(string , int = 0, bool = false, bool = false, bool = false);
    int getCWsize();

    static void * checktimeout(void *temp_arg) {
      thread_args *t_arg = (thread_args *)temp_arg;
      tcp *object = (tcp *)t_arg->object_pointer;
      int seqNumsent = t_arg->seqNum;

      sleep(TIMEOUT);
      if(seqNumsent > object->recvack)
      {
	pthread_mutex_lock(&(object->timeoutlock));	
	if(object->packetTimeout)
	{
	  pthread_mutex_unlock(&(object->timeoutlock));
	  return NULL;
	}
	object->packetTimeout = true;	
	pthread_mutex_unlock(&(object->timeoutlock));	
      }
      return NULL;
    }

    static void * dummyReceiveLoop(void *object) {
      ((tcp *)object)->receiveLoop();
      return NULL;
    }
};
