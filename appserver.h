#include "Bank.h"

typedef struct account{
	pthread_mutex_t lock;
	int value;
} account;

typedef struct request{
	char *command;
	struct timeval timeStart;
	int requestId;
	struct request *next;
} request;

typedef struct queue{
	int count;
	request* front;
	request* rear;
} queue;

void push(char* cmd, int requestId);
request pop();
