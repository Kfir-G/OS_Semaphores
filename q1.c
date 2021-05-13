#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> //for I/O 
#include <stdlib.h> //for rand/strand 
#include <windows.h> //for Win32 API 
#include <time.h> //for using the time as the //seed to strand!
#define BUFFER_SIZE 5 //Size Of buffer
#define MAX_SLEEP_MSEC 5000 //Maximum sleep time in //miliseconds
#define MAX_PROD_CONS 100 //Maximum //Producer/Consumer Array size
#define INIT_SEM_COUNT 0
#define RAND_MAX 1000 
#define True 1 
#define False 0


//define the insert and remove functions 
int insert_item(int item); //returns True/False 
int remove_item(int* item); //returns True/False
void initArray(int*, int);

//define the Producer/Consumer Thread functions
DWORD WINAPI Producer(LPVOID Param);
DWORD WINAPI Consumer(LPVOID Param);

HANDLE emptySlots = BUFFER_SIZE;
HANDLE fullSlots = 0;
HANDLE ghMutex = 1;
HANDLE ProducerThreadHandles[MAX_PROD_CONS];
HANDLE ConsumerThreadHandles[MAX_PROD_CONS];
int buffer[BUFFER_SIZE];
// Definition of 3 Semaphores should also be here!
int in = 0; //next empty slot in buffer to put an
int out = 0; //next full slot to take an item from 
int count = 0; //count of current full slots in buffer


int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("error - Enter 2 parameters only");
		exit(0);
	}
	int producerSize = atoi(argv[1]);
	int comsumerSize = atoi(argv[2]);
	
	if (((producerSize < 1 && producerSize > MAX_PROD_CONS) || (comsumerSize < 1 && comsumerSize > MAX_PROD_CONS)))
	{
		printf("error - ");
		exit(0);
	}

	


	emptySlots =
		CreateSemaphore(NULL,//default security attributes
			BUFFER_SIZE, // initial count
			BUFFER_SIZE, // maximum count
			NULL); // unnamed semaphore
	fullSlots =
		CreateSemaphore(NULL,//default security attributes
			0, // initial count
			BUFFER_SIZE, // maximum count
			NULL); // unnamed semaphore

	ghMutex = CreateSemaphore(NULL, // default security attributes
		1, // initially not owned
		1,NULL);

	if (ghMutex == NULL || emptySlots == NULL || fullSlots == NULL) {
		printf("Create Semaphore error: %d\n", GetLastError());
		return 1;
	}
	DWORD test; //
	initArray(buffer, BUFFER_SIZE);


	int producerID[MAX_PROD_CONS];
	for (int i = 0; i < producerSize; i++)
	{
		producerID[i] = i;
		ProducerThreadHandles[i] = CreateThread(NULL, 0, Producer, &producerID[i], 0, &test);
		if (ProducerThreadHandles[i] == NULL)
		{
			printf("main: Unexpected Error in producer thread %d creation", i);
			return 1;
		}
	}
	DWORD test1; //
	int consumerID[MAX_PROD_CONS];
	
	for (int i = 0; i < comsumerSize; i++)
	{
		consumerID[i] = i;
		ConsumerThreadHandles[i] = CreateThread(NULL, 0, Consumer, &consumerID[i], 0, &test1);
		if (ConsumerThreadHandles[i] == NULL)
		{
			printf("main: Unexpected Error in Consumer thread %d creation", i);
			return 1;
		}
	}
	//join on threads

	WaitForMultipleObjects(producerSize, ProducerThreadHandles, TRUE, INFINITE);
	WaitForMultipleObjects(comsumerSize, ConsumerThreadHandles, TRUE, INFINITE);

	CloseHandle(ghMutex);
	CloseHandle(fullSlots);
	CloseHandle(emptySlots);

	for (int i = 0; i < producerSize; i++)
	{
		CloseHandle(ProducerThreadHandles[i]);
	}

	for (int i = 0; i < comsumerSize; i++)
	{
		CloseHandle(ConsumerThreadHandles[i]);
	}

}

int insert_item(int item) {

	WaitForSingleObject(emptySlots, INFINITE);//No Timeout Inter.
	WaitForSingleObject(ghMutex, INFINITE);//No Timeout Inter.
	if (buffer[in] != -1) {
		printf("error - cell is not empty");
		return False;
	}

	else {
		++count;
		printf("Producer produced: %d count: %d", item, count);
		buffer[in] = item;
		in = (in + 1) % BUFFER_SIZE;
		
		
		if (!ReleaseSemaphore(ghMutex,// handle to Semaphore,
			1, // increase count by one
			NULL)) // ptr to Sem�s previous value
		{
			printf("ReleaseSemaphore error: %d\n", GetLastError());
		}
		if (!ReleaseSemaphore(fullSlots,// handle to Semaphore,
			1, // increase count by one
			NULL)) // ptr to Sem�s previous value
		{
			printf("ReleaseSemaphore error: %d\n", GetLastError());
		}
		return True;
	}
}

int remove_item(int* item) {

	WaitForSingleObject(fullSlots, INFINITE);//No Timeout Inter.
	WaitForSingleObject(ghMutex, INFINITE);//No Timeout Inter.
	if (buffer[out] == -1) {
		printf("error - cell is not empty");
		return False;
	}
	else
	{
		*item = buffer[out];
		buffer[out] = -1;
		--count;
		out = (out + 1) % BUFFER_SIZE;
		
		printf("Consumer Consumed: %d count: %d", *item, count);

		if (!ReleaseSemaphore(ghMutex,// handle to Semaphore,
			1, // increase count by one
			NULL)) // ptr to Sem�s previous value
		{
			printf("ReleaseSemaphore error: %d\n", GetLastError());
		}
		if (!ReleaseSemaphore(emptySlots,// handle to Semaphore,
			1, // increase count by one
			NULL)) // ptr to Sem�s previous value
		{
			printf("ReleaseSemaphore error: %d\n", GetLastError());
		}
		return True;
	}
}
void initArray(int* arr, int size) {
	for (int i = 0; i < size; i++)
	{
		arr[i] = -1;
	}
}
DWORD WINAPI Producer(LPVOID Param) {
	while (True)
	{
		int Prducer_id = *(int*)Param;
		srand(time(NULL));
		int timeToSleep = rand() % MAX_SLEEP_MSEC + 1;
		Sleep(timeToSleep);
		int itemToInsert = rand() % RAND_MAX + 1;

		if (insert_item(itemToInsert)) {
			printf("Prducer_id = %d and Producer Item = %d\n", Prducer_id, timeToSleep);
		}
		else
		{
			printf("error - insert_item");
		}



	}
	return False;
}


DWORD WINAPI Consumer(LPVOID Param) {
	int Consumer_id = *(int*)Param;

	while (True)
	{
		int timeToSleep = rand() % MAX_SLEEP_MSEC + 1;
		Sleep(timeToSleep);

		int refitem;
		if (remove_item(&refitem)) {
			printf("Consumer_id = %d and Consumed Item = %d\n", Consumer_id, refitem);
		}
		else
		{
			printf("error - remove_item");
		}
	}
	

	return False;
}
