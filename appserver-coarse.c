#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "appserver.h"


//Delete this later, this is for testing the queue
void display(request *head);

//Function to setup the queue
void queueInit();

//This is the function that processes the users commands stored in the queue
void * processCmd();

queue *q;
account *accounts;
pthread_mutex_t queueMutex;
pthread_mutex_t bankLock;
FILE *output;

int id = 1;
int running =1;
int i;

int main (int argc, char *argv[]){

	//Check for valid arguments to the program
	if(argc != 4){
		printf("Launch the server with the following syntax\n");
		printf("./appserver <# of worker thread> <# of accounts> <output file>\n");
		exit(1);
	}

	//Setup the queue
	q = (queue*) malloc(sizeof(queue));
	queueInit();
	pthread_mutex_init(&queueMutex, NULL);
	
	//Set the number of treads and accounts according to the arguments
	int workerThreads = atoi(argv[1]);
	int numAccounts = atoi(argv[2]);
	char outName[strlen(argv[3]+1)];
	strcpy(outName, argv[3]);
	outName[strlen(argv[3])] = '\0';

	output= fopen(outName, "w");
	
	//Allocate memory for the accounts
	accounts = (account*) malloc(numAccounts*sizeof(account));

	//Setup the accounts
	initialize_accounts(numAccounts);
	for(i=0; i<numAccounts; i++){
		pthread_mutex_init(&(accounts[i].lock), NULL);
		accounts[i].value = 0;
	}

	pthread_t threads[workerThreads];
	for(i=0; i<workerThreads; i++){
		pthread_create(&threads[i], NULL, processCmd, NULL);
	}
	
	pthread_mutex_init(&bankLock, NULL);
	printf("Works fine up to the main server loop\n");

	//Main server loop that does everthing
	while(running){
		
		printf("> ");
		
		char request[1024];
    	fgets(request, 1024, stdin);
		request[strlen(request)-1] = '\0';
		
		if((strcmp(request, "END")) == 0){
			running = 0;
			break;
		}

		pthread_mutex_lock(&queueMutex);
		push(request, id);
		printf("< ID %d\n", id);
		pthread_mutex_unlock(&queueMutex);
		id++;
    }

	for(i=0; i<workerThreads; i++){
		pthread_join(threads[i], NULL);
	}
	fclose(output);
	return 0;

}

void queueInit(){
	q->front = NULL;
	q->rear = NULL;
	q->count =0;
}

void display(request *head){
	if(head == NULL){
		printf("NULL\n");
	} else {
		printf("%s\n", head->command);
		display(head->next);
	}
}

void push(char *cmd, int requestId){
	request *toAdd = malloc(sizeof(request));
	
	toAdd->command = malloc(1024*sizeof(char));
	strncpy(toAdd->command, cmd, 1024);
	toAdd->requestId = requestId;
	gettimeofday(&(toAdd->timeStart),NULL);
	toAdd->next = NULL;

	if(q->count > 0){
		q->rear->next = toAdd;
		q->rear = toAdd;
		q->count = q->count+1;
	} else {
		q->front = toAdd;
		q->rear = toAdd;
		q->count = 1;
	}
}

request pop(){
	request *temp;
	request toPop;

	if(q->count >0){
		toPop.requestId = q->front->requestId;
		toPop.command = malloc(1024 * sizeof(char));
		toPop.timeStart = q->front->timeStart;
		strncpy(toPop.command, q->front->command, 1024);
		toPop.next = NULL;

		temp = q->front;
		q->front = q->front->next;
		free(temp->command);
		free(temp);
		
		if(!q->front){
			q->rear = NULL;
		}

		q->count = q->count -1;
	} else {
		toPop.command = NULL;
	}

	return toPop;
}

void * processCmd(){
	while(running || q->front != NULL){
		pthread_mutex_lock(&queueMutex);
		if(q->front != NULL){
			request req;
			req = pop();
			pthread_mutex_unlock(&queueMutex);

			//Create a string array of the request parameters
			int arrayLen = strlen(req.command);
			char charArray[arrayLen+1];
			strcpy(charArray, req.command);
			charArray[arrayLen = '\0'];
			int length = strlen(charArray);
			int j;
    			int spaces =0;
    			for(j =0; j<length; j++){
      				if(charArray[j] == ' '){
        				spaces++;
      				}
    			}

    			//Initialize the argument array to have space for a null at the end
    			char* command[(spaces+2)];
    			//Initialize the entire array to be null to start
    			for(j =0; j<spaces+2; j++){
      				command[j] = NULL;
    			}

    			int i = 0;

    			//Cut the string string up into individual array indexes
    			char * cut = strtok(charArray, " ");
    			while(cut != NULL){
        			command[i] = cut;
        			cut = strtok(NULL, " ");
        			i++;
    			}

			//If the request is a check request
			if(strcmp(command[0], "CHECK") == 0){
				int balance;
				int accountNum = atoi(command[1]);
				pthread_mutex_lock(&bankLock);
				balance = read_account(accountNum);
				pthread_mutex_unlock(&bankLock);
				struct timeval finished;
				gettimeofday(&finished, NULL);

				fprintf(output, "%d BAL %d TIME %d.%06d %d.%06d\n", req.requestId, balance, req.timeStart.tv_sec, req.timeStart.tv_usec, finished.tv_sec, finished.tv_usec);
			}
			else if((strcmp(command[0], "TRANS")) == 0){
				int numOfTrans = spaces/2;
				int accountNums[numOfTrans];
				int amounts[numOfTrans];
				int ISF=0;
				
				int i;
				int accIndex =0;
				int amIndex =0;
				//Loop through the command array, starting at 1
				//Odd indexes should be the account num, and even should be the ammounts
				pthread_mutex_lock(&bankLock);
				for(i=1; i<spaces+1; i++){
					if(i%2 == 0){
						amounts[amIndex] = atoi(command[i]);
						amIndex++;
					}
					else{
						accountNums[accIndex] = atoi(command[i]);
						accIndex++;
					}
				}

				for(i=0; i<numOfTrans; i++){
					int accBalance = read_account(accountNums[i]);
					if((accBalance + amounts[i]) < 0){
						ISF=1;
						break;
					}
				}
				if(ISF){
					struct timeval finished;
					gettimeofday(&finished, NULL);
					fprintf(output, "%d ISF %d TIME %d.%06d %d.%06d\n", req.requestId, accountNums[i], req.timeStart.tv_sec, req.timeStart.tv_usec, finished.tv_sec, finished.tv_usec);
				}
				else{
					for(i=0; i< numOfTrans; i++){
						account acc = accounts[accountNums[i]-1];
						int accBalance = read_account(accountNums[i]);
						write_account(accountNums[i], (accBalance+amounts[i]));
					}
					struct timeval finished;
					gettimeofday(&finished, NULL);
					fprintf(output, "%d OK TIME %d.%06d %d.%06d\n", req.requestId, req.timeStart.tv_sec, req.timeStart.tv_usec, finished.tv_sec, finished.tv_usec);
				}
				pthread_mutex_unlock(&bankLock);
			}
		} else {
			pthread_mutex_unlock(&queueMutex);
		}

	}
}
