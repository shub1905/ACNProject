#include "udp_to_tcp.h"

int main(int argc, char **argv)
{
  tcp u;
  u.ip = argv[1];
  u.establish();
  cerr << "Client Connection Established" << endl;
  string something;
  int bytes_received = 0;
  while(true) 
  {
    bytes_received = u.receive(something);
    if(something.find("\04") != string::npos)
      break;
    cout << something;
    cout << flush;
  }
  cerr << "Client Receive Complete" << endl;
}
