#include "udp_to_tcp.h"

int main(int argc, char **argv)
{
  tcp u;
  u.ip = argv[1];
  u.establish();
  cout << "Client Connection Established" << endl;
  string something;
  int bytes_received = 0;
  int counter;
  int i;
  while(true) 
  {
    bytes_received = u.receive(something);
    cout << something << endl;
  }
  while(true);
}
