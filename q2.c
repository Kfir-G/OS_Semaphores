#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define THREAD_NUM 5 //Number of Threads (Trains!)
#define LANE_NUM 3//Number of Lanes in Train Interchange
#define MAX_TRAIN_RIDES 5 
#define SLEEP_TIME 500 //Miliseconds (1/2 second)
#define True 1
#define False 0

//Definition of Lane Interchange DB should also be	here!
int lanes[LANE_NUM];

//Definition of 2 Semaphores should also be here!
HANDLE ResourceSemphore;
HANDLE mutex;

int out = 0;
int count = 0;

int enterTrainInterchange(int trainID);
int exitTrainInterchange(int lane, int trainID);

DWORD WINAPI runTrain(PVOID Param); //Thread Func.
void initArray(int*, int);


int main(int argc, char* argv[])
{
	HANDLE trainsArr[THREAD_NUM];
	DWORD train_ID;

	//Initialise Semaphores
	mutex = CreateSemaphore(NULL, 1, 1, NULL);
	if (mutex == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		return True;
	}
	ResourceSemphore = CreateSemaphore(NULL, LANE_NUM, LANE_NUM, NULL);
	if (ResourceSemphore == NULL)
	{
		printf("main: Unexpected Error in ResourceSemphore creation");
		return True;
	}

	//Initialise Interchange Lanes DB
	initArray(lanes, LANE_NUM);

	//Initialise Train Threads Array
	int train_Treads[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; i++)
	{
		train_Treads[i] = i;
		trainsArr[i] = CreateThread(NULL, 0, runTrain, &train_Treads[i], 0, &train_ID);
		if (trainsArr[i] == NULL)
		{
			printf("main: Unexpected Error in enter trains thread %d creation", i);
			return 1;
		}
	}

	//Join on both all Train Threads
	WaitForMultipleObjects(THREAD_NUM, trainsArr, TRUE, INFINITE);

	//Output a Message that All trains had finished!
	printf("All Trains are Completed!!");

	//Close all handles properly!
	CloseHandle(mutex);
	CloseHandle(ResourceSemphore);

	for (int j = 0; j < THREAD_NUM; j++)
	{
		CloseHandle(trainsArr[j]);
	}

	//return False;
}

void initArray(int* arr, int size) {
	for (int i = 0; i < size; i++)
	{
		arr[i] = -1;
	}
}

DWORD WINAPI runTrain(PVOID Param)
{
	int lane;
	int trainID = *(int*)Param;

	for (int i = 0; i < MAX_TRAIN_RIDES; i++)
	{

		lane = enterTrainInterchange(trainID);

		if (lane == -1)
		{
			printf("error Unexpected Error thread %d Entering\n", trainID);
			return False;
		}
		else
		{
			printf("Train = %d Enter Lane = %d\n", trainID, lane);
			Sleep(SLEEP_TIME);
			printf("Enter: Train = %d Exit Lane = %d\n", trainID, lane);

			if (!exitTrainInterchange(lane, trainID))
			{
				printf("error Unexpected Error thread %d Exiting\n", trainID);
				return 1;
			}
		}


	}
	return 0;
}

int enterTrainInterchange(int trainID)
{
	int i;
	WaitForSingleObject(ResourceSemphore, INFINITE);
	WaitForSingleObject(mutex, INFINITE);

	/*Access DB in Mutual-Exclusion:
	Find an available Lane and update DB to associate
	trainID with this Lane – turning it into an occupied Lane!*/
	for (i = 0; i < LANE_NUM; i++)
	{
		if (lanes[i] == -1)
		{

			lanes[i] = trainID;
			break;
		}
	}

	if (i == LANE_NUM)
	{

		if (!ReleaseSemaphore(mutex, 1, NULL)) {
			printf("ReleaseSemaphore mutex error: %d\n", GetLastError());
		}
		if (!ReleaseSemaphore(ResourceSemphore, 1, NULL)) {
			printf("ReleaseSemaphore ResourceSemphore error: %d\n", GetLastError());
		}
		printf("Error!");
		return -1;
	}
	if (!ReleaseSemaphore(mutex, 1, NULL)) {
		printf("ReleaseSemaphore mutex error: %d\n", GetLastError());
	}

	return i;
}


int exitTrainInterchange(int lane, int trainID)
{
	WaitForSingleObject(mutex, INFINITE);
	out = lane;
	if (lanes[out] != trainID)
	{
		printf("Error!, In lane: %d the trainId is: %d and not %d\n", out, lanes[out], trainID);
		if (!ReleaseSemaphore(mutex, 1, NULL)) {
			printf("ReleaseSemaphore error: %d\n", GetLastError());
		}
		return False;
	}
	lanes[out] = -1;
	if (!ReleaseSemaphore(mutex, 1, NULL)) {
		printf("ReleaseSemaphore error: %d\n", GetLastError());
	}
	if (!ReleaseSemaphore(ResourceSemphore, 1, NULL)) {
		printf("ReleaseSemaphore error: %d\n", GetLastError());
	}
	return True;
}
