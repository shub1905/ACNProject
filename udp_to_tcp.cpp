class tcp
{
  void listen()
  {
    int sd, rc, n;
    char msg[MAX_MSG];
    struct sockaddr_in selfAddress;

    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0) {
      printf("Cannot open socket \n");
      exit(1);
    }

    this->selfAddr.sin_family = AF_INET;
    this->selfAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->selfAddr.sin_port = htons(LOCAL_SERVER_PORT);
    rc = bind (sd, (struct sockaddr *) &selfAddr,sizeof(selfAddr));
    if(rc<0) {
      printf("Cannot bind port number %d \n", LOCAL_SERVER_PORT);
      exit(1);
    }
    printf("Waiting for data on port UDP %u\n", LOCAL_SERVER_PORT);
  }
  
  int establish()
  {
    int sd, rc, i;
    struct sockaddr_in cliAddr;
    struct hostent *h;

    h = gethostbyname(this->ip.c_str());
    if(h==NULL) {
      printf("Unknown host '%s' \n", this->ip.c_str());
      exit(1);
    }

    printf("Sending data to '%s' (IP : %s) \n", h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

    this->remoteAddress.sin_family = h->h_addrtype;
    memcpy((char *) &this->remoteAddress.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
    this->remoteAddress.sin_port = htons(REMOTE_SERVER_PORT);

    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0) {
      printf("Cannot open socket \n");
      exit(1);
    }

    /* bind any port */
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
    if(rc<0) {
      printf("Cannot bind port\n");
      exit(1);
    }
  }

  bool receivePacket(string &data)
  {
    /* server infinite loop */
    while(1) {

      /* init buffer */
      memset(msg,0x0,MAX_MSG);


      /* receive message */
      cliLen = sizeof(cliAddr);
      n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &cliAddr, &cliLen);

      if(n<0) {
	printf("Cannot receive data \n");
	continue;
      }

      /* print received message */
      printf("%s: from %s:UDP%u : %s \n",argv[0],inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port),msg);
      int rc = sendto(sd,msg, strlen(msg)+1, 0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
      if (rc < 0) {
	printf("client : cannot send data \n");
	close(sd);
	exit(1);
      }

    }/* end of server infinite loop */
  }
}
