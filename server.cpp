#include "udp_to_tcp.h"

int main()
{
  struct timeval start_time,end_time;
  double total_time = 0;
  tcp t;
  string arbit;
  string baap;

  t.listen();
  cerr << "Server Connection Established" << endl;
  getline(cin,arbit);
  arbit.append("\n");
  t.send(arbit);
  baap.reserve(700*1000000);
  cerr << "String length "<< baap.length() << "\t" << baap.capacity() << endl;
  while(getline(cin, arbit)) {
    arbit += "\n";
    baap.append(arbit);
  }
  cerr << "Generated the string. Sending it now :)" << endl;
  gettimeofday(&start_time,NULL);
  t.send(baap);
  gettimeofday(&end_time,NULL);
  total_time = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);
  arbit = "\04";
  t.send(arbit);
  cerr << "The send was successful" << endl;
  cerr << "Total Time : " << total_time/1000000.0 << endl;
  exit(0);
}
