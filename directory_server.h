#include "common.h"
#define MAXLINELENGTH 30
#define MINVALUE 9999
struct docList
{
	int docId,no_of_servers;
	struct fileserverList *fsList;
};

struct fileserverList 
{
	int fileserverId;
	struct fileserverList *next;

};
struct docList list[2];
void registration();
void resource_discovery();
void checkResource(int);
int findClosestFileServer(int);
int getserverid(char *);
int docpresent(char *, int);

