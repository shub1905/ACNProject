#include <string>
#include <iostream>

class tcp
{
  string ip;
  int port;
  int socket;
  sockaddr_in remoteAddress;
  bool connectionEstablished;

  int establish();
  bool kill();
  int send(string &data);
  int receive(string &data);
  bool sendPacket(string &data);
  bool receivePacket(string &data);
}
