#include "udp_to_tcp.h"

int main()
{
  tcp t;
  t.listen();
  cout << "DONE SERVER" << endl;
  string tmp = "what the fuck is wrong with this piece of shit?";
  string arbit;
  for(int i=0;i<10000;i++) {
    arbit = arbit+tmp;
  }
  //cout<< arbit << endl;
  t.send(arbit);
  cout << "the send was successful" << endl;
  while(true);
}
