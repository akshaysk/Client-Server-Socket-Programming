#include "directory_server.h"
/* This function processes registration requests from file servers. */
void registration()
{
	int fd,len;
	struct sockaddr_in dirAddr, srcAddr;
	struct hostent *directory_ip_addr;
	struct in_addr *address;
	char buffer[BUFSIZE];
	int i=2,serverid;
	char *occurrence;
	FILE *fp;

	socklen_t addrlen = sizeof(srcAddr);

	memset(buffer,0,BUFSIZE);
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		perror("Phase 1: Error: Cannot create socket of directory server");
	}

	memset((char *)&dirAddr, 0, sizeof(dirAddr)); 
	dirAddr.sin_family = AF_INET; 
	
	directory_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)directory_ip_addr->h_addr;

	dirAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	dirAddr.sin_port = htons(DIRECTORY_UDP_PORT_1);
	if (bind(fd, (struct sockaddr *)&dirAddr, sizeof(dirAddr)) < 0) 
	{ 
		perror("Phase 1: Error: Directory server bind failed");
	}

	printf("\nPhase 1: The Directory Server has UDP port number %d and IP address %s\n",htons(dirAddr.sin_port), inet_ntoa(*address));
	truncate("directory.txt",0);
	do
	{
		occurrence = NULL;
		while((len = recvfrom(fd, buffer, BUFSIZE, 0, (struct sockaddr *)&srcAddr, &addrlen))<0)
		{
			printf("Phase 1: Error in receiving registration data");
			continue;
		}
		if((occurrence = strstr(buffer,"Server1"))!=NULL){
			serverid = 1;}
		else if((occurrence = strstr(buffer,"Server2"))!=NULL){
			serverid = 2;}
		else if((occurrence = strstr(buffer,"Server3"))!=NULL){
			serverid = 3;}
		else{
			printf("Phase 1: Data received from an invalid file server");
			close(fd);
			exit(0);
		}	
		printf("\nPhase 1: The Directory Server has received request from File Server%d\n",serverid);

		if((fp = fopen("directory.txt","a+")) == NULL){
			perror("\nPhase 1: Error opening directory.txt\n");
			close(fd);
			exit(0);
		}

		fprintf(fp,"%s\n",buffer);
		fclose(fp);
	}while(i--);

	printf("\nPhase 1: The directory.txt file has been created\n\nPhase 1: End of Phase 1 for the directory server\n");



}
/* Receive client's requests and find the nearest file server for that client*/
void resource_discovery()
{
	int fd,len;
	struct sockaddr_in dirAddr, srcAddr;
	struct hostent *directory_ip_addr;
	struct in_addr *address;
	char buffer[BUFSIZE];
	int i=1,clientid,closest_file_server;
	char *occurrence;
	char file_server[MAXLINELENGTH], my_message[BUFSIZE];
	int serverid = 0, port_no=0;
	FILE *fp;

	socklen_t addrlen = sizeof(srcAddr);

	memset(buffer,0,BUFSIZE);
	memset(my_message,0,BUFSIZE);
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		perror("Phase 2: Error: Cannot create socket of directory server");
		exit(0);
	}

	memset((char *)&dirAddr, 0, sizeof(dirAddr)); 
	dirAddr.sin_family = AF_INET; 
	
	directory_ip_addr = gethostbyname("nunki.usc.edu");
	address = (struct in_addr *)directory_ip_addr->h_addr;

	dirAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*address));
	
	dirAddr.sin_port = htons(DIRECTORY_UDP_PORT_2);
	if (bind(fd, (struct sockaddr *)&dirAddr, sizeof(dirAddr)) < 0) 
	{ 
		perror("Phase 2: Error: Directory server bind failed");
		exit(0);
	}

	printf("\nPhase 2: The Directory Server has UDP port number %d and IP address %s\n",htons(dirAddr.sin_port), inet_ntoa(*address));
	
	do
	{
	occurrence = NULL;
	while((len = recvfrom(fd, buffer, BUFSIZE, 0, (struct sockaddr *)&srcAddr, &addrlen))<0)
	{
		printf("Phase 2: Error in receiving client request");
		continue;
	}
	if((occurrence = strstr(buffer,"Client1"))!=NULL){
		clientid = 1;}
	else if((occurrence = strstr(buffer,"Client2"))!=NULL){
		clientid = 2;}
	else{
		printf("Phase 2: Data received from an invalid client");
		close(fd);
		exit(0);
	}	
	printf("\nPhase 2: The Directory Server has received request from client%d\n",clientid);
	
	checkResource(clientid);
	if(list[clientid-1].no_of_servers == 1)
	{
		closest_file_server = list[clientid-1].fsList->fileserverId;
	}
	else if(list[clientid-1].no_of_servers > 1 )
	{
		closest_file_server = findClosestFileServer(clientid);
	}
	if((fp = fopen("directory.txt","r")) == NULL){
		perror("\nPhase 2: Error opening directory.txt\n");
		close(fd);
		exit(0);
	}

	while(fscanf(fp,"%s %s %d",file_server,file_server,&port_no))
	{
		serverid = getserverid(file_server);	
		if(closest_file_server == serverid)
			break;
	}
	
	snprintf(my_message,BUFSIZE,"File_server%d %d",closest_file_server,port_no);
	if(sendto(fd, my_message, strlen(my_message), 0, (struct sockaddr *)&srcAddr, sizeof(srcAddr)) < 0) 
	{
		perror("Phase 2: Error in sendto() for file server");
		close(fd);
		exit(0);
	}
	printf("\nPhase 2: File Server details has been sent to client%d\n",clientid);
	}while(i--);

	
	printf("\nPhase 2: End of Phase 2 for the Directory Server\n");
}

/* This function checks for the nearest file server using topology.txt*/
int findClosestFileServer(int clientid)
{
	FILE *fp;
	char line[MAXLINELENGTH];
	int i = clientid, minValue = MINVALUE, minServer=0;
	int dist[3];
	struct fileserverList *fslist=NULL, *p=NULL;

	if((fp = fopen("topology.txt","r")) == NULL){
		perror("\nError opening resource.txt\n");
		exit(0);
	}

	while(i--)
	{
		fgets(line,sizeof(line),fp);
	}

	sscanf(line,"%d %d %d",&dist[0],&dist[1],&dist[2]);

	fslist = list[clientid-1].fsList;
	p = fslist;
	while(p!=NULL)
	{
		if(dist[p->fileserverId-1]<minValue)
		{
			minValue = dist[p->fileserverId-1];
			minServer = p->fileserverId;
		}
		p = p->next;
	}	


	return minServer; 
		
}

int getserverid(char *line)
{
	if(strstr(line,"Server1")!=NULL)
		return 1;
	else if(strstr(line,"Server2")!=NULL)
		return 2;
	else if(strstr(line,"Server3")!=NULL)
		return 3;

	return -1;

}	

int docpresent(char *line, int clientid)
{
	switch(clientid)
	{
		case 1:
			if(strstr(line,"doc1")!=NULL)
				return 1;
			else return 0;
			break;
		case 2:	
			if(strstr(line,"doc2")!=NULL)
				return 1;
			else return 0;
			break;
		default: 
			return 0;
	}

	return 0;
}

/*Check if the file server has the said resource*/
void checkResource(int docid)
{
	FILE *fp;
	char line[MAXLINELENGTH];
	int serverid = 0;
	struct fileserverList *head = NULL, *p = NULL, *q = NULL;
	
	if((fp = fopen("resource.txt","r")) == NULL){
		perror("\nError opening resource.txt\n");
		exit(0);
	}

	list[docid-1].docId = docid;
	while(fgets(line,sizeof(line),fp)!=NULL)
	{
		if(!docpresent(line,docid))
			continue;
		serverid = getserverid(line);	
		list[docid-1].no_of_servers++;
		p = (struct fileserverList *)malloc(sizeof(struct fileserverList));
		if(p == NULL)
		{
			perror("Error allocating file server objects");exit(0);
		}

		p->fileserverId = serverid;
		p->next = NULL;
	
		if(head == NULL)
		{
			head = p;
		}
		else
		{
			q = head;
			while(q->next != NULL)
				q = q->next;
			q->next = p; 
		}

	}

	list[docid-1].fsList = head;

	fclose(fp);
}


int main()
{
	registration();
	resource_discovery();
	return 0;
}
