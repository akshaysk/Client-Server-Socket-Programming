#include "client.h"
/* The client opens up udp connection and get the file server's TCP port number*/
void clientFunction(int n)
{
	struct sockaddr_in clientAddr,destAddr,srcAddr;
	int fd, port_no,len, tcp_port_no;
	char my_message[BUFSIZE], buffer[BUFSIZE],file_server[BUFSIZE];
	struct hostent *client_ip_addr, *directory_ip_addr;
	struct in_addr * address;
	socklen_t addrlen;
	memset(my_message,0,BUFSIZE);
	memset(buffer,0,BUFSIZE);

	if(n != 1 && n != 2)
	{
		perror("\nPhase 2: Invalid parameter passed while creating client\n");
		exit(0);
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		perror("Phase 2: Error cannot create client server socket\n");
		exit(0);
	}
	memset((char *)&clientAddr, 0, sizeof(clientAddr)); 
	clientAddr.sin_family = AF_INET; 
	client_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)client_ip_addr->h_addr;

	clientAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	if(n == 1) 
		port_no = CLIENT_1_UDP_PORT;
	else if(n == 2) 
		port_no = CLIENT_2_UDP_PORT;

	clientAddr.sin_port = htons(port_no);

	if(bind(fd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) 
	{ 
		perror("\nPhase 2: Client bind failed\n");
		close(fd);
		exit(0);
	}

	printf("\nPhase 2: Client %d has UDP port number %d and IP address %s\n",n,htons(clientAddr.sin_port), inet_ntoa(*address));
	
	memset((char *)&destAddr, 0, sizeof(destAddr)); 
	destAddr.sin_family = AF_INET; 
 	directory_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)directory_ip_addr->h_addr;

	destAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	destAddr.sin_port = htons(DIRECTORY_UDP_PORT_2);

	snprintf(my_message,BUFSIZE,"Client%d doc%d",n,n);	
	if(sendto(fd, my_message, strlen(my_message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0) 
	{
		perror("Phase 1: Error in sendto() for client server");
		close(fd);
		exit(0);
	}

	printf("\nPhase 2: The File request from Client %d has been sent to the Directory Server\n",n);
	while((len = recvfrom(fd, buffer, BUFSIZE, 0, (struct sockaddr *)&srcAddr, &addrlen))<0)
	{
		printf("Phase 1: Error in receiving registration data");
		continue;
	}
	sscanf(buffer,"%s %d",file_server,&tcp_port_no);
	printf("\nPhase 2: The File doc%d requested by Client %d is present in %s and the File Server's TCP port number is %d\n",n,n,file_server,tcp_port_no);
	printf("\nPhase 2: End of Phase 2 for Client %d\n",n);
	tcpclient(n,tcp_port_no,file_server);
}

/* The client will establish a TCP client and will request the file resource to the file server*/
void tcpclient(int clientid, int file_server_tcp_port, char *file_server)
{
	int fd;
	struct sockaddr_in file_addr, my_addr;
	struct hostent *file_ip_addr;
	struct in_addr * address;
	socklen_t alen = sizeof(file_addr);
	
	char my_message[BUFSIZE], recv_message[BUFSIZE];
	memset(my_message, 0, BUFSIZE);
	memset(recv_message, 0, BUFSIZE);
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		perror("cannot create socket"); 
		exit(0); 
	}

	memset((char*)&file_addr, 0, sizeof(file_addr)); 
	file_addr.sin_family = AF_INET; 
	file_addr.sin_port = htons(file_server_tcp_port); 
	file_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)file_ip_addr->h_addr;

	file_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	if (connect(fd, (struct sockaddr *)&file_addr, alen) == -1) {
		close(fd);
		perror("client: connect");
		exit(0);
	}

	if(getsockname(fd,(struct sockaddr *)&my_addr,&alen) == -1)
	{
		perror("getsockname");
		close(fd);
		exit(0);
	}

	printf("\nPhase 3: Client %d has dynamic TCP port number %d and IP address %s\n", clientid,my_addr.sin_port, inet_ntoa(*(struct in_addr *)(gethostbyname("nunki.usc.edu")->h_addr)));
	snprintf(my_message,BUFSIZE,"Client%d doc%d",clientid,clientid);
	if(write(fd, my_message, BUFSIZE) < 0) 
	{
		perror("Phase 1: Error in send() for client");
		close(fd);
		exit(0);
	}
	printf("\nPhase 3: The File request from Client %d has been sent to the %s\n",clientid,file_server);

	if(read(fd, recv_message, BUFSIZE) < 0)
	{
		printf("Phase 3: Error in receiving data from tcp client");
		close(fd);
		exit(0);
	}
	printf("\nPhase 3: Client %d received %s from %s\n",clientid,recv_message,file_server);
	printf("\nPhase 3: End of Phase 3 for Client %d\n",clientid);
}

/*Kill the outstanding file server processes which are not yet terminated*/
void kill_prev_procs()
{
	FILE *fp = fopen("procs","r");
	int ret;
	int f1, f2, f3, f4;
	if((ret = fscanf(fp,"%d %d %d %d\n",&f1,&f2,&f3,&f4)) == 4)
	{	
		kill(f1,SIGINT);
		kill(f2,SIGINT);
		kill(f3,SIGINT);
		kill(f4,SIGINT);
	}
	fclose(fp);	
}

int main()
{
	pid_t client1, client2;

	client1 = fork();
	if(client1 == 0)
		clientFunction(1);
	else 
	{	
		client2 = fork();
		if(client2 == 0)
			clientFunction(2);
		else
		{
			wait(NULL);
			wait(NULL);
			kill_prev_procs();
		}
	}
	return 0;
}
