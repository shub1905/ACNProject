#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

main() {
	FILE *f = fopen("100mb_file", "r");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *str = (char *)malloc(fsize + 1);
	fread(str, fsize, 1, f);
	fclose(f);

	str[fsize] ='\0';
	string str1(str,str+fsize);
	cout<<str1.length()<<endl;
}
