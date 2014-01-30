class tcp
{
  void listen()
  {
    int sd, rc, n;
    struct sockaddr_in servAddr;
    char msg[MAX_MSG];

    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0) {
      printf("%s: cannot open socket \n",argv[0]);
      exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(LOCAL_SERVER_PORT);
    rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
    if(rc<0) {
      printf("%s: cannot bind port number %d \n",argv[0], LOCAL_SERVER_PORT);
      exit(1);
    }
    printf("%s: waiting for data on port UDP %u\n", argv[0],LOCAL_SERVER_PORT);
  }
  
  int establish()
  {
    int sd, rc, i;
    struct sockaddr_in cliAddr, remoteServAddr;
    struct hostent *h;

    /* check command line args */
    if(argc<3) {
      printf("usage : %s <server> <data1> ... <dataN> \n", argv[0]);
      exit(1);
    }

    /* get server IP address (no check if input is IP address or DNS name */
    h = gethostbyname(argv[1]);
    if(h==NULL) {
      printf("%s: unknown host '%s' \n", argv[0], argv[1]);
      exit(1);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

    remoteServAddr.sin_family = h->h_addrtype;
    memcpy((char *) &remoteServAddr.sin_addr.s_addr,h->h_addr_list[0], h->h_length);
    remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0) {
      printf("%s: cannot open socket \n",argv[0]);
      exit(1);
    }

    /* bind any port */
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
    if(rc<0) {
      printf("%s: cannot bind port\n", argv[0]);
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
	printf("%s: cannot receive data \n",argv[0]);
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
