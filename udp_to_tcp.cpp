#include "udp_to_tcp.h"

void tcp::listen()
{
  int sd, rc, n;
  struct sockaddr_in selfAddress;
  char msg[MAX_MSG];
  memset(msg,0,MAX_MSG);
  unsigned int len = sizeof(this->remoteAddress);

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
  header = *(tcp_header *)msg;

  if((header.permissions2 & 16) == 16)
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
  if (setsockopt (this->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
  {
    cerr << "setsockopt failed" << endl;
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
  cout << "in get next packet" << endl;
  rc = recvfrom(this->sock, msg, MAX_MSG, 0, (struct sockaddr *) &this->remoteAddress, &len);
  if(rc<0)
  {
    cout << "retrying send syn" << endl;
    goto retry_send_syn;
  }
  tcp_header header;
  header = *(tcp_header *)msg;

  cout << "what?" << header.length << endl;
  if((header.permissions2 & 16) == 16 && (header.permissions2 & 2) == 2)
  {
    cout << "question" << endl;
    this->sendack = header.seqNum - 1;
    this->sendPacket("ACK PACKET",this->seqnumber + 1);
    this->connectionEstablished = true;
  }
  else
  {
    goto get_next_packet;
  }
  return this->sock;
}

tcp::tcp()
{
  this->connectionEstablished = false;
  pthread_mutex_init(&acklock,NULL);
  pthread_mutex_init(&pktloss,NULL);
  pthread_mutex_init(&timeoutlock,NULL);
}

int tcp::getCWsize()
{
  return CWSIZE;
}

int tcp::send(string &data)
{
  int acknum,bytestosend,origseqnum = seqnumber;
  pthread_mutex_lock(&acklock);
  datatosend = data.length();
  pthread_mutex_unlock(&acklock);
  for(int i=0;i<data.length();) {
    bool retransmit = false;
    pthread_mutex_lock(&timeoutlock);
    if(packetTimeout){
      int lastackdata = recvack - origseqnum;
      i = lastackdata;
      packetTimeout = false;
      retransmit = true;
    }
    pthread_mutex_unlock(&timeoutlock);

    pthread_mutex_lock(&pktloss);
    if(numacks >= 3) {
      int lastackdata = recvack - origseqnum;
      i = lastackdata;
      retransmit = true;
    }
    pthread_mutex_unlock(&pktloss);

    bytestosend = min(data.length()-i,PACKETSIZE);

    pthread_mutex_lock(&acklock);
    if(retransmit){
      seqnumber  =recvack;
    }
    if( (seqnumber - recvack ) > getCWsize()){
      pthread_mutex_unlock(&acklock);
      continue;
    }
    datatosend-=bytestosend;
    if(sendack){
      acknum = sendack;
      sendack = 0;
    }
    else{
      acknum = 0;
    }
    pthread_mutex_unlock(&acklock);
    if(sendPacket(data.substr(i,bytestosend), acknum)){
      i+=bytestosend;
    }
    else{
      datatosend+=bytestosend;
    }
  }
  return data.length();
}

bool tcp::sendPacket(string data,int acknum, bool syn,bool fin,bool retransmission)
{
  char * buf = new char[sizeof(tcp_header)+data.length()+1];
  memset(buf,0,sizeof(tcp_header));
  tcp_header * header = (tcp_header *)buf;
  header->length = sizeof(tcp_header) + data.length();
  header->seqNum = seqnumber;
  header->ackNum = acknum;
  header->permissions2|= (acknum>0 ? (16)  : 0);
  header->permissions2|= ( syn ?(2) : 0);
  header->permissions2|= (fin ? (1) :0  );
  strncpy(buf+sizeof(tcp_header),data.c_str(),data.length()); /*Assumes '\0' is appended*/

  int packetsize = header->length + 1 ;
  //	assert packetsize (<UDPPACKETSIZE);
  int bytessend = sendto(sock, buf , packetsize, 0,(struct sockaddr *) &remoteAddress,sizeof(remoteAddress));
  delete buf;

  thread_args t_arg;

  if (bytessend == packetsize){
    seqnumber+=( retransmission ? 0: data.length()  ) ;
    if(data.length()){
      pthread_t *timeoutthread = new pthread_t();
      t_arg.object_pointer= this;
      t_arg.seqNum = header->seqNum;
      pthread_create(timeoutthread,NULL,checktimeout,&t_arg);
    }
    return true;
  }
  return false;
}

long long min(long long x, long long y)
{
  return (x<y)?x:y;
}
