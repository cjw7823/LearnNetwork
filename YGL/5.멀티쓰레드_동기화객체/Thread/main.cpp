#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <time.h>


unsigned WINAPI ThreadIncrease(void* arg);
unsigned WINAPI ThreadDecrease(void* arg);

int Gold = 0;

CRITICAL_SECTION cs;
HANDLE MutexHandle;
//accept
//clientSocket -> vector

int main()
{
	HANDLE ThreadHandle[20];//다루기 쉽게 프로세스마다 핸들 값을 따로 준다.
	unsigned int threadID;//쓰레드의 메모리 주소(절대주소)가 들어갈 공간.
	//핸들 값은 프로세스간에 동일할 수 있다.(분명 다른 쓰레드임에도 불구하고.) 하지만 쓰레드의 절대 주소는 같을 수 없다.(실제 메모리의 주소이므로)

	clock_t startTime;
	clock_t endTime;

	startTime = clock();

	//CriticalSection 초기화
	//InitializeCriticalSection(&cs);
	//Mutex
	WCHAR MutexName[] = L"무조건더하기";
	MutexHandle = CreateMutex(NULL, FALSE, MutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("이미 실행 중입니다..\n");
		return -1;
	}

	//Sleep(30000);

	for (int i = 0; i < 20; ++i)
	{
		if (i % 2 == 0)
		{
			ThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadIncrease, NULL, 0, &threadID);
		}
		else
		{
			ThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadDecrease, NULL, 0, &threadID);
		}
	}


	WaitForMultipleObjects(20, ThreadHandle, TRUE, INFINITE);

	printf("Gold : %d\n", Gold);

	//DeleteCriticalSection(&cs);
	CloseHandle(MutexHandle);

	endTime = clock();

	printf("elapsed time : %f\n", (double)(endTime - startTime));

	return 0;
}

unsigned WINAPI ThreadIncrease(void* arg)
{

	//EnterCriticalSection(&cs);
	WaitForSingleObject(MutexHandle, INFINITE);
	for (int i = 0; i < 100000000; ++i)
	{

		Gold = Gold + 1;

	}
	//LeaveCriticalSection(&cs);
	ReleaseMutex(MutexHandle);

	return 0;
}

unsigned WINAPI ThreadDecrease(void* arg)
{
	//EnterCriticalSection(&cs);
	WaitForSingleObject(MutexHandle, INFINITE);
	for (int i = 0; i < 100000000; ++i)
	{

		Gold = Gold - 1;

	}
	//LeaveCriticalSection(&cs);
	ReleaseMutex(MutexHandle);



	return 0;
}

