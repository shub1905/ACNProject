#include "udp_to_tcp.h"

int main()
{
  tcp t;
  t.listen();
  cout << "Server Connection Established" << endl;
  string tmp = "This is a hello world kind of string";
  string arbit;
      for(int i=0;i<10000;i++) {
      arbit = arbit+tmp;
    }
  for(int j=0;j<100;j++) { 
    t.send(arbit);
  }
  cout << "The send was successful" << endl;
  while(true);
}
