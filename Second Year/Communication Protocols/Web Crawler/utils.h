#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <map>
#define BUFLEN 4096
#define MAX_CLIENTS 10
#define PORT 80
using namespace std;

//Print status and error messages
void print(string msg, FILE* out){
	if(out)
		fprintf(out, "%s", (msg + "\n").c_str());
	else
		cout << msg << "\n";
}
//Print error messages
void error(string msg, FILE* err){
	print(msg, err);
	exit(0);
}