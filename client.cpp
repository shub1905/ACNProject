#include "udp_to_tcp.h"

int main(int argc, char **argv)
{
  tcp u;
  u.ip = argv[1];
  u.establish();
  cout << "Client Connection Established" << endl;
  string something;
  while(true) 
  {
    u.receive(something);
  }
  while(true);
}
