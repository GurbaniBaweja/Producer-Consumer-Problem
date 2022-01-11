#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <semaphore.h>
#include "tands.h"

using namespace std;

//Reference for global output file: https://stackoverflow.com/questions/3327751/creating-a-global-file-object/3327766
FILE *output_file; 
sem_t mutex;
sem_t emp;
sem_t full;
bool completed_execution = true;

int work, ask, receive, complete, sleep = 0;  //summary variables
int *twork_done;  //array to determine #T executed by each thread, reference: https://stackoverflow.com/questions/3851181/define-array-then-change-its-size

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Implementation of queue
// Code adapted from : https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/ and modified. Code discovered with Chanpreet Singh
class Queue {
public:
	int front, rear, size;
	unsigned capacity;
	string* array;
};

// function to create a queue of given capacity.
Queue* createQueue(unsigned capacity)
{
	Queue* queue = new Queue();
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;
	queue->array = new string[queue->capacity];
	return queue;
}

// Queue is full when size becomes equal to the capacity
string isFull(Queue* queue)
{
    string result;
    if(queue->size == queue->capacity){
        return "fullQ";
    }
}

// Queue is empty when size is 0
string isEmpty(Queue* queue)
{
    if(queue->size == 0){
        return "emptyQ";
    }
}

// Function to add an item to the queue.
void enqueue(Queue* queue, string item)
{
	if (isFull(queue) == "fullQ")
		return;
	queue->rear = (queue->rear + 1)
				% queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
}

// Function to remove an item from queue.
string dequeue(Queue* queue)
{
	if (isEmpty(queue) == "emptyQ")
		return "emptyQ";
	string item = queue->array[queue->front];
	queue->front = (queue->front + 1)
				% queue->capacity;
	queue->size = queue->size - 1;
	return item;
}

// Function to get front of queue
string front(Queue* queue)
{
	if (isEmpty(queue) == "emptyQ")
		return "emptyQ";
	return queue->array[queue->front];
}

// Function to get rear of queue
string rear(Queue* queue)
{
	if (isEmpty(queue) == "emptyQ")
		return "emptyQ";
	return queue->array[queue->rear];
}
// Creating a global shared queue for producer and consumer
Queue* sharedQ ;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Producer functionality, syncronization implemented using semaphores
void* Producer (void* vargp)
{
    string input;
    //Taking input for I/O redirection
    while(cin>>input){
        //Wait 
        sem_wait(&emp);  //decrements empty semaphore
        sem_wait (&mutex);  //enter mutex

        //Parent receives work
        work++;
        fprintf(output_file,"ID=0 Work %d \n",input[1]);
        
        //Critical Section
        if(input[0] =='T'){
            enqueue(sharedQ,input);  //Inserting the input to shared queue
        }
        else{
            //Parent sleeps
            fprintf(output_file,"ID=0 Sleep %d \n",input[1]);
            char num = input[1];
            Sleep(int(num));  //Implementing Sleep()
            sleep++;
        }
        
        //Signal
        sem_post(&mutex);  //leave mutex
        sem_post(&full);  //increments full semphore
        
    }completed_execution = false;
    return NULL;    
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Consumer  functionality, implemented using semaphores
void* Consumer (void* args_p)
{
    int* thread_location = (int *) args_p;
    int thread_id = (*thread_location) + 1;  //As ID = 0 belongs to the Producer
    int work_done = 0;  // #T executed by each thread
    
    while(completed_execution){//Parent receives work 
        //Consumer thread asks for work
        ask++;
        fprintf(output_file,"ID=%d Ask \n",thread_id);
        
        //wait
        sem_wait(&full); //decrements full semaphore
        sem_wait (&mutex);  //enter mutex
        
        //Critical Section
        string command = dequeue(sharedQ);  //Taking out the command from shared queue
        char num = command[1];
        //Consumer thread receives work
        receive++;
        fprintf(output_file,"ID=%d Receive %e \n",thread_id,num);
        Trans(int(num));   //Implementing Trans()
        
        fprintf(output_file,"ID=%d Complete %e \n",thread_id,num);
        complete++;
        twork_done[thread_id]++;
        //signal
        sem_post(&mutex);  //leave mutex
        sem_post(&emp); //increments empty
    }
    
    return NULL;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------


int main (int argc, char* argv[]) {

    int nthreads = atoi(argv[1]);
    int Qsize = 2 * nthreads;  //Queue size twice of consumer threads
    sharedQ = createQueue(Qsize);  //Allocating a size to the global queue

    twork_done = new int[nthreads];   //array to determine #T executed by each thread, Reference: https://stackoverflow.com/questions/3851181/define-array-then-change-its-size
    
    //Checking for the id for output file and creating the file
    if (argc >2){
        string id = argv[2];
        const char *outputFile = ("prodcon."+id+ ".log").c_str();;
        output_file = fopen("outputFile","w+");
    }
    else{
        output_file = fopen("Prodcon.log","w+");
    }
    
    //Creating semaphores
    sem_init(&mutex,1,0);      
    sem_init(&emp, 1,Qsize);  
    sem_init(&full, 1, 0);    

    //Creating producer and consumer threads 
    pthread_t producerThread_id;
    pthread_t consumerThread_id[nthreads];

    for (int numOfthread = 0; numOfthread <= nthreads; numOfthread++){
        if (numOfthread == 0){
            int return_thread = pthread_create(&producerThread_id, NULL, Producer, NULL);
            if (return_thread != 0){
                exit(EXIT_FAILURE);   //failed thread creation
            }
        }
        else{
            int return_thread = pthread_create(&consumerThread_id[numOfthread], NULL, Consumer, &numOfthread);
            if (return_thread != 0){
                exit(EXIT_FAILURE);   //failed thread creation
            }
        }
    }

    //Waiting for threads to terminate
    for (int numOfthread = 0; numOfthread <= nthreads; numOfthread++){
        if(numOfthread ==0){
            pthread_join(producerThread_id,NULL);
        }
        else {
            pthread_join(consumerThread_id[numOfthread], NULL);
        }
    }
    //Destroying the semaphores
    sem_destroy(&mutex);
    sem_destroy(&emp);
    sem_destroy(&full);
    
    //Writing the summary 
    fprintf(output_file,"Summary: \n");
    fprintf(output_file, "    Work%d \n",work);
    fprintf(output_file, "    Ask%d \n",ask);
    fprintf(output_file, "    Receive%d \n",receive);
    fprintf(output_file, "    Complete%d \n",complete);
    fprintf(output_file, "    Sleep%d \n",sleep);

    for (int numOfthread = 0; numOfthread < nthreads; numOfthread++){
        fprintf(output_file, "    Thread%d \n",(numOfthread+1),twork_done[numOfthread]);
    }

    fclose(output_file);
    return 0; 
}
