#include "udp_to_tcp.h"

int main()
{
  tcp u;
  u.ip = "localhost";
  u.establish();
  cout << "done client" << endl;
}
