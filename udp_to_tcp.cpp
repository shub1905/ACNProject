#include "udp_to_tcp.h"

int main()
{
  cout << "Compiled" << endl;
}

void tcp::listen()
{
  int sd, rc, n;
  struct sockaddr_in selfAddress;

  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("Cannot open socket \n");
    exit(1);
  }

  selfAddress.sin_family = AF_INET;
  selfAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  selfAddress.sin_port = htons(LOCAL_SERVER_PORT);
  rc = bind (sd, (struct sockaddr *) &selfAddress,sizeof(selfAddress));
  if(rc<0) {
    printf("Cannot bind port number %d \n", LOCAL_SERVER_PORT);
    exit(1);
  }
  this->sock = sd;

  /* SERVER SIDE HANDSHAKE */
retry_receive:
  rc = recvfrom(this->sock, msg, MAX_MSG, 0, (struct sockaddr *) &this->remoteAddress, &len);
  if(rc<0)
  {
    goto retry_receive;
  }
  tcp_header header;
  header = *(tcp_header *)msg;

  this->seqnumber = (rand()%10)*100;
  this->sendack = header.seqNum;
  this->sendPacket("SYN+ACK PACKET",this->sendack+1,true,false);

  rc = recvfrom(this->sock, msg, MAX_MSG, 0, (struct sockaddr *) &this->remoteAddress, &len);
  if(rc<0)
  {
    cout << "SYN chutiyapa" << endl;
    exit(1);
  }
  header = *(tcp_header *)recv_packet;

  if((header.permissions2 | 16) == 1)
  {
    this->connectionEstablished = true;
  }
  else
  {
    cout << "SYN chutiyapa unexpected" << endl;
    exit(1);
  }
}

int tcp::establish()
{
  int sd, rc, i;
  struct sockaddr_in cliAddr;
  struct hostent *h;
  char msg[MAX_MSG];
  memset(msg,0,MAX_MSG);
  unsigned int len = sizeof(this->remoteAddress);

  h = gethostbyname(this->ip.c_str());
  if(h==NULL) 
  {
    cout << this->ip << "Unknown Host" << endl;
    exit(1);
  }

  this->remoteAddress.sin_family = h->h_addrtype;
  memcpy((char *) &this->remoteAddress.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
  this->remoteAddress.sin_port = htons(REMOTE_SERVER_PORT);

  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0)
  {
    printf("Cannot open socket \n");
    exit(1);
  }

  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);

  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0)
  {
    printf("Cannot bind port\n");
    exit(1);
  }
  this->sock = sd;

  struct timeval timeout;      
  timeout.tv_sec = TIMEOUT_VAL;
  timeout.tv_usec = 0;
  if (setsockopt (this->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
  {
    error("setsockopt failed\n");
  }

  /* CLIENT SIDE HANDSHAKE IMPLEMENTATION
   * send syn
   * receive syn+ack
   * send ack
   */
  this->seqnumber = (rand()%10)*100;
retry_send_syn:
  this->sendPacket("SYN PACKET",0,true,false);
get_next_packet:
  rc = recvfrom(this->sock, msg, MAX_MSG, 0, (struct sockaddr *) &this->remoteAddress, &len);
  if(rc<0)
  {
    goto retry_send_syn;
  }
  tcp_header header;
  header = *(tcp_header *)msg;

  if((header.permissions2 | 16) == 1 && (header.permissions2 | 2) == 1)
  {
    this->sendack = header.seqnumber - 1;
    this->sendPacket("ACK PACKET",this->seqnumber + 1);
    this->connectionEstablished = true;
  }
  else
  {
    goto get_next_packet;
  }
  return this->sock;
}
