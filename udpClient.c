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

#define REMOTE_SERVER_PORT 1500
#define MAX_MSG 100

int main(int argc, char *argv[]) {

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


	/* send data */
	while(1) {
		printf("type data to send\n");
		char buff[128];
		scanf("%s",buff);
		if (strcmp(buff,"exit") == 0) {
			break;
		}
		rc = sendto(sd, buff, strlen(buff)+1, 0,(struct sockaddr *) &remoteServAddr,sizeof(remoteServAddr));
		if(rc<0) {
			printf("%s: cannot send data %d \n",argv[0],i-1);
			close(sd);
			exit(1);
		}
		char msg[128];
		unsigned int serLen = sizeof(remoteServAddr);
		int n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &remoteServAddr,&serLen);

		if(n<0) {
			printf("%s: cannot receive data \n",argv[0]);
			continue;
		}
		printf("data received = %s\n",msg);


	}
	return 1;
}
