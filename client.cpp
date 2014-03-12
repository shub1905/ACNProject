#include "udp_to_tcp.h"

int main()
{
  tcp u;
  u.ip = "localhost";
  u.establish();
  cout << "done client" << endl;
  string something;
  while(true) 
  {
    u.receive(something);
  }
  while(true);
}
