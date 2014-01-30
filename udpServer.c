#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <pthread.h>

#define LOCAL_SERVER_PORT 1500
#define MAX_MSG 100

int sd, rc, n, cliLen;
struct sockaddr_in cliAddr, servAddr;
char msg[MAX_MSG];
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * sendfunc() {
	while(1) {
		pthread_cond_signal(&cond2);
		pthread_cond_wait(&cond1,&mutex);
		int rc = sendto(sd,msg, strlen(msg)+1, 0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
		if (rc < 0) {
			printf("client : cannot send data \n");
			close(sd);
			exit(1);
		}
	}

}

int main(int argc, char *argv[]) {


	/* socket creation */
	sd=socket(AF_INET, SOCK_DGRAM, 0);
	if(sd<0) {
		printf("%s: cannot open socket \n",argv[0]);
		exit(1);
	}

	/* bind local server port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(LOCAL_SERVER_PORT);
	rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
	if(rc<0) {
		printf("%s: cannot bind port number %d \n",argv[0], LOCAL_SERVER_PORT);
		exit(1);
	}

	printf("%s: waiting for data on port UDP %u\n", argv[0],LOCAL_SERVER_PORT);

	pthread_t sender;
	pthread_create(&sender,NULL,sendfunc,NULL);
	/* server infinite loop */
	while(1) {

		/* init buffer */
		pthread_cond_wait(&cond2,&mutex);
		memset(msg,0x0,MAX_MSG);

		cliLen = sizeof(cliAddr);
		n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &cliAddr, &cliLen);

		if(n<0) {
			printf("%s: cannot receive data \n",argv[0]);
			continue;
		}
		printf("%s: from %s:UDP%u : %s \n",argv[0],inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port),msg);
		pthread_cond_signal(&cond1);

	}/* end of server infinite loop */

	return 0;

}
