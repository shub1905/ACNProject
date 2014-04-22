#include "udp_to_tcp.h"
#define DEBUG false
#define KNOWL true

void tcp::listen() {
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
  this->recvack = this->seqnumber;
  this->seqnumberRemote = header.seqNum;
  this->remoteBaseSeqNumber = header.seqNum;
  this->sendPacket("SYN+ACK PACKET",this->remoteBaseSeqNumber+1,true,false);

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
    cout << "SYN Chutiyapa unexpected" << endl;
    exit(1);
  }
  pthread_t* receive_thread = new pthread_t();
  pthread_create(receive_thread,NULL,dummyReceiveLoop,this);
}

void tcp::receiveLoop() {
  //use 3rd last bit of permissions2 for EOF flag	
  //tcp_header.length = length of total packet
  //tcp class contains seqNum

  char buffer[MAX_MSG];
  tcp_header header;
  int lengthDataRecv = 0;

  while(1) {
    memset(buffer,0x0,MAX_MSG);
    if (receivePacket(buffer)) {
      memcpy((char *)&header,buffer,sizeof(tcp_header));

      if ((header.permissions2 & 0x10) == 0x10) {
	pthread_mutex_lock(&pktloss);
	if (header.ackNum > this->recvack) {
	  this->recvack = header.ackNum;
	  this->numacks = 1;
	} else if (header.ackNum == this->recvack){
	  this->numacks++;
	}
	pthread_mutex_unlock(&pktloss);
      }

      lengthDataRecv = header.length - sizeof(tcp_header);
      if (lengthDataRecv > 0 ) {

	int diff = (head - tail + BUF_SIZE_OS - 1)%BUF_SIZE_OS;
	if (header.seqNum - this->seqnumberRemote >= diff) {
	  if(KNOWL)
	    cout << endl << "Dropping this packet" << endl;
	  continue; //packet intentionally dropped outside of buffer
	}
	for(int i=0;i<lengthDataRecv;i++) {
	  this->bitmapReceive[(header.seqNum + BUF_SIZE_OS + i  - (this->remoteBaseSeqNumber + lengthDataRecv))%BUF_SIZE_OS] = true;
	  this->dataBuffer[(header.seqNum + BUF_SIZE_OS + i  - (this->remoteBaseSeqNumber + lengthDataRecv))%BUF_SIZE_OS] = buffer[sizeof(tcp_header)+i];
	}
	
	while(bitmapReceive[tail]) {
	  tail = (tail+1)%BUF_SIZE_OS;
	  this->seqnumberRemote++;
	}
	//protected access
	pthread_mutex_lock(&acklock);
	if (datatosend > 0) {
	  sendack = this->seqnumberRemote;
	  pthread_mutex_unlock(&acklock);
	}
	else {
	  pthread_mutex_unlock(&acklock);
	  sendPacket("",header.seqNum);
	}
	//access ends
      }
      if ((header.permissions2 & 0x4) == 1) {
	break;//do something else; designated eofile
      }
    }
  }
}

int tcp::receive(string &data) {
  int orig_tail = tail;
  int totalread = (orig_tail +BUF_SIZE_OS - head) % BUF_SIZE_OS;
  for(int i=0;i<totalread;i++) {
    data[i] = dataBuffer[(head+i) % BUF_SIZE_OS];
    this->bitmapReceive[(head+i) % BUF_SIZE_OS] = false;
  }
  head=orig_tail;
  return totalread;
}

bool tcp::receivePacket(char *data) {
  unsigned int remoteLen = sizeof(this->remoteAddress);
  int n = recvfrom(sock, data, sizeof(int), MSG_PEEK, (struct sockaddr *) &this->remoteAddress,&remoteLen);
  int total_packet_size = *(int *)data;
  n = recvfrom(sock,data,total_packet_size,0,(struct sockaddr *) &this->remoteAddress,&remoteLen);
  tcp_header *temp_header = (tcp_header *)data;
  int data_len = temp_header->length - sizeof(tcp_header);
  if(DEBUG)
    printf("packet recvd : %s\n",data+sizeof(tcp_header));
  if(KNOWL)
    cout << "Packet Received. Bytes recvd " << data_len << endl;

  if (n<=0) {
    return false;
  }
  return true;
}

int tcp::establish() {
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
  this->recvack = this->seqnumber;
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

  if((header.permissions2 & 16) == 16 && (header.permissions2 & 2) == 2)
  {
    this->sendack = header.seqNum;
    this->remoteBaseSeqNumber = this->sendack;
    this->sendPacket("ACK PACKET",this->seqnumber);
    this->connectionEstablished = true;
  }
  else
  {
    goto get_next_packet;
  }
  pthread_t* receive_thread = new pthread_t();
  pthread_create(receive_thread,NULL,dummyReceiveLoop,this);
  return this->sock;
}

tcp::tcp() {
  this->connectionEstablished = false;
  pthread_mutex_init(&acklock,NULL);
  pthread_mutex_init(&pktloss,NULL);
  pthread_mutex_init(&timeoutlock,NULL);
  memset(this->bitmapReceive,false,BUF_SIZE_OS);
  this->head=this->tail=0;
  this->datatosend = 0;
  this->sendack = 0; //to set
  this->recvack = 0;
  this->numacks = 0;
  this->packetTimeout = false;
}

int tcp::getCWsize() {
  return CWSIZE;
}

int tcp::send(string &data) {
  int acknum,bytestosend,origseqnum = seqnumber;
  int i=0;
  pthread_mutex_lock(&acklock);
  datatosend = data.length();
  pthread_mutex_unlock(&acklock);
  while((recvack < seqnumber) || (i<data.length())) {
    bool retransmit = false;
    pthread_mutex_lock(&timeoutlock);
    if(packetTimeout){
      if(KNOWL)
	cout << "Packet is time-outing " << i<< " "<< recvack<< endl;
      int lastackdata = recvack - origseqnum;
      i = lastackdata;
      packetTimeout = false;
      retransmit = true;
    }
    pthread_mutex_unlock(&timeoutlock);

    pthread_mutex_lock(&pktloss);
    if(this->numacks >= 3) {
      if(KNOWL)
	cout << "Triple DUP-ACK Loss" << endl;
      int lastackdata = recvack - origseqnum;
      i = lastackdata;
      retransmit = true;
    }
    pthread_mutex_unlock(&pktloss);

    bytestosend = min(data.length()-i,PACKETSIZE);

    pthread_mutex_lock(&acklock);
    if(retransmit){
      seqnumber  = recvack;
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
    if(DEBUG)
      cout << "i" << i << endl;
    if(sendPacket(data.substr(i,bytestosend), acknum)){
      i+=bytestosend;
    }
    else{
      datatosend+=bytestosend;
    }
  }
  return data.length();
}

bool tcp::sendPacket(string data,int acknum, bool syn,bool fin,bool retransmission) {
  char * buf = new char[sizeof(tcp_header)+data.length()+1];
  memset(buf,0,sizeof(tcp_header));
  tcp_header * header = (tcp_header *)buf;
  header->length = sizeof(tcp_header) + data.length();
  seqnumber += ( retransmission ? 0: data.length()  ) ;
  header->seqNum = seqnumber;
  header->ackNum = acknum;
  header->permissions2|= (acknum>0 ? (16)  : 0);
  header->permissions2|= ( syn ?(2) : 0);
  header->permissions2|= (fin ? (1) :0  );
  strncpy(buf+sizeof(tcp_header),data.c_str(),data.length()); /*Assumes '\0' is appended*/

  int packetsize = header->length + 1 ;
  //	assert packetsize (<UDPPACKETSIZE);
  int bytessend = sendto(sock, buf , packetsize, 0,(struct sockaddr *) &remoteAddress,sizeof(remoteAddress));
  if(KNOWL)
    cout << "Packet sent. Bytes sent : " << data.length() << endl;

  thread_args *t_arg = new thread_args();

  if (bytessend == packetsize){
    if(data.length()){
      pthread_t *timeoutthread = new pthread_t();
      t_arg->object_pointer= this;
      t_arg->seqNum = header->seqNum;
      pthread_create(timeoutthread,NULL,checktimeout,t_arg);
    }
    delete buf;
    return true;
  } else {
    seqnumber -= ( retransmission ? 0: data.length()  ) ;
  }
  delete buf;
  return false;
}

long long min(long long x, long long y) {
  return (x<y)?x:y;
}
