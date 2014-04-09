#define _POSIX_C_SOURCE_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#define BUFSIZE 25

#define DIRECTORY_UDP_PORT_1	21089
#define DIRECTORY_UDP_PORT_2	31089

#define FILE_SERVER_1_UDP_PORT	22089
#define FILE_SERVER_1_TCP_PORT	41089

#define FILE_SERVER_2_UDP_PORT	23089
#define FILE_SERVER_2_TCP_PORT	42089

#define FILE_SERVER_3_UDP_PORT	24089
#define FILE_SERVER_3_TCP_PORT	43089

#define CLIENT_1_UDP_PORT	32089
#define CLIENT_2_UDP_PORT	33089

