#include "file_server.h"
/*Start the registration process with the directory server*/
void registration(int n)
{
	struct sockaddr_in fileAddr,destAddr;
	int fd, udp_port_no,tcp_port_no;
	char my_message[BUFSIZE];
	struct hostent *file_ip_addr, *directory_ip_addr;
	struct in_addr * address;
	memset(my_message,0,BUFSIZE);

	if(n != 1 && n != 2 && n!= 3)
	{
		perror("\nPhase 1: Invalid parameter passed while creating file server\n");
		exit(0);
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		perror("Phase 1: Error cannot create file server socket\n");
		exit(0);
	}
	memset((char *)&fileAddr, 0, sizeof(fileAddr)); 
	fileAddr.sin_family = AF_INET; 
	file_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)file_ip_addr->h_addr;

	fileAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	if(n == 1) 
	{
		udp_port_no = FILE_SERVER_1_UDP_PORT;
		tcp_port_no = FILE_SERVER_1_TCP_PORT;
	}
	else if(n == 2) 
	{
		udp_port_no = FILE_SERVER_2_UDP_PORT;
		tcp_port_no = FILE_SERVER_2_TCP_PORT;
	}
	else if(n == 3) 
	{	
		udp_port_no = FILE_SERVER_3_UDP_PORT;
		tcp_port_no = FILE_SERVER_3_TCP_PORT;
	}
	fileAddr.sin_port = htons(udp_port_no);
	
	if(bind(fd, (struct sockaddr *)&fileAddr, sizeof(fileAddr)) < 0) 
	{ 
		perror("\nPhase 1: File server bind failed\n");
		close(fd);
		exit(0);
	}

	printf("\nPhase 1: File Server %d has UDP port number %d and IP address %s\n",n,htons(fileAddr.sin_port), inet_ntoa(*address));
	
	memset((char *)&destAddr, 0, sizeof(destAddr)); 
	destAddr.sin_family = AF_INET; 
 	directory_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)directory_ip_addr->h_addr;

	destAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	destAddr.sin_port = htons(DIRECTORY_UDP_PORT_1);

	snprintf(my_message,BUFSIZE,"File Server%d %d",n,tcp_port_no);
	if(sendto(fd, my_message, strlen(my_message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0) 
	{
		perror("Phase 1: Error in sendto() for file server");
		close(fd);
		exit(0);
	}

	printf("\nPhase 1: The Registration request from File Server %d has been sent to the Directory Server\n",n);
	printf("\nPhase 1: End of Phase 1 for File Server %d\n",n);
}

/*This function will create a TCP server and accept the client's requests*/
void tcpserverfunction(int serverid)
{
	int fd, tcp_port,rqst,len;
	struct hostent *file_ip_addr;
	struct in_addr * address;
	struct sockaddr_in my_addr; 
	socklen_t alen = sizeof(my_addr);
	char buffer[BUFSIZE],my_message[BUFSIZE], client_name[BUFSIZE],doc_name[BUFSIZE];

	memset(my_message,0,BUFSIZE);
	memset(buffer,0,BUFSIZE);
	memset(client_name,0,BUFSIZE);
	memset(doc_name,0,BUFSIZE);
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		perror("cannot create socket"); 
		exit(0); 
	}

	if(serverid == 1)
		tcp_port = FILE_SERVER_1_TCP_PORT;
	else if(serverid == 2)
		tcp_port = FILE_SERVER_2_TCP_PORT;
	else if(serverid == 3)
		tcp_port = FILE_SERVER_3_TCP_PORT;

	memset((char*)&my_addr, 0, sizeof(my_addr)); 
	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(tcp_port); 
	file_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)file_ip_addr->h_addr;

	my_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	if (bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) 
	{
		perror("bind failed"); 
		close(fd);
		exit(0); 
	}
	printf("\nPhase 3: File Server %d has TCP port %d and IP address %s\n",serverid,htons(my_addr.sin_port),inet_ntoa(*address));
	
	if (listen(fd,10) < 0) 
	{ 
		perror("listen failed"); 
		close(fd);
		exit(0); 
	}
	while(1)
	{
		if((rqst = accept(fd, (struct sockaddr *)NULL, &alen)) < 0) 
		{ 
			perror("accept failed"); close(fd);exit(0); 
		}


		if((len = read(rqst, buffer, BUFSIZE)) < 0)
		{
			printf("Phase 3: Error in receiving data from tcp client");
			close(fd);
			exit(0);
		}

		sscanf(buffer,"%s %s",client_name,doc_name);

		printf("\nPhase 3: File Server %d received the request from %s for the file %s\n",serverid, client_name, doc_name);

		if(write(rqst, doc_name, BUFSIZE) < 0) 
		{
			perror("Phase 1: Error in sendto() for client server");
			close(fd);
			exit(0);
		}
		printf("\nPhase 3: File Server %d has sent %s to %s\n",serverid, doc_name, client_name);
	}
}

void print_pids(int file_server1, int file_server2, int file_server3)
{	
	int parent = (int)getpid();
	truncate("procs",0);
	FILE *fp = fopen("procs","w");
	fprintf(fp,"%d %d %d %d\n",file_server1,file_server2,file_server3, parent);
	fclose(fp);	
}

int main()
{
	pid_t file_server1, file_server2, file_server3;

	file_server1 = fork();
	if(file_server1 == 0)
	{	
		registration(1);
		tcpserverfunction(1);
	}
	else
	{
		file_server2 = fork();

		if(file_server2 == 0)
		{
			registration(2);
			tcpserverfunction(2);
		}
		else
		{
			file_server3 = fork();
			if(file_server3 == 0)
			{
				registration(3);
				tcpserverfunction(3);
			}
			else
			{
				print_pids(file_server1, file_server2, file_server3);
				wait(NULL);	
			}
		}
	}
	return 0;
}
