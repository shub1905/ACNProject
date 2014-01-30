#define MAX_MSG 100
#define BUF_SIZE_OS 1000

char dataBuffer[BUF_SIZE_OS];
int readp=0,recvp=0;

bool receivePacket(string &data) {
	int remoteLen = sizeof(remoteAddress);
	int n = recvfrom(sock,data,MAX_MSG,0,(struct sockaddr *) &remoteAddress,&remoteLen);
	if (n<0) {
		return false;
	}
	return true;

}
///////////////////////////////////////BITMAP Functions//////////////////////////////////////////
char bitmap[BUF_SIZE_OS];
bool bitmapSet(char *bitmap, int pos, bool p, int size = BUF_SIZE_OS) {
	if (pos >= size) {
		return false;
	}
	int a = pos/8;
	int b = pos%8;
	char c = (char)(p<<b);
	*(bitmap+a) |= c;
	return true;
}
void bitmapreset(char *bitmap, int size = BUF_SIZE_OS) {
	memset(bitmap,0x0,size);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool receive(string &data) {
	//use 3rd last bit of permissions2 for EOF flag	
	//tcp_header.length = length of total packet
	//tcp class contains seqNum
	
	char buffer[MAX_MSG];
	tcp_header header;
	int data_received_till = seqNum;
	int ackRecv = 0;
	int lengthDataRecv = 0;

	while(1) {
		memset(buffer,0x0,MAX_MSG);
		if (receivePacket(buffer)) {
			memcpy((char *)&header,buffer,sizeof(tcp_header));

			if ((header.permissions2 & 0x10) == 1) {
				pthread_mutex_lock(&acklock);
				if (header.ackNum > recvack) {
					recvack = header.ackNum;
				}
				pthread_mutex_unlock(&acklock);
			}

			lengthDataRecv = header.length - sizeof(tcp_header);
			if (lengthDataRecv > 0 ) {
				int diff = recvp - readp;//adjust initial values and add mod here
				if (header.seqNum - seqNum >diff) {
					continue; //packet intentionally dropped outside of buffer
				}
				memcpy(dataBuffer + recvp + header.seqNum - seqNum, buffer + sizeof(tcp_header),lengthDataRecv);
				if (data_received_till == header.seqNum) {
					data_received_till += lengthDataRecv;
				}

				//protected access
				pthread_mutex_lock(&acklock);
				if (datatosend > 0) {
					sendack = data_received_till;
					pthread_mutex_unlock(&acklock);
				}
				else {
					pthread_mutex_unlock(&acklock);
					sendPacket(NULL,data_received_till);
				}
				//access ends
			}
			if ((header.permissions2 & 0x4) == 1) {
				break;//do something else; designated eofile
			}
		}
	}
}

int readRec(string &data) {
    int origrecvp =recvp;
    int totalread = (origrecvp+BUF_SIZE_OS - readp) % BUF_SIZE_OS ;
    for(int i=0;i<totalread;i++)
   	 data[i] = dataBuffer[(readp+i) % BUF_SIZE_OS];
    readp=origrecvp;
    return totalread;
}