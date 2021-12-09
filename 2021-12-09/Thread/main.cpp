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
	HANDLE ThreadHandle[20];//�ٷ�� ���� ���μ������� �ڵ� ���� ���� �ش�.
	unsigned int threadID;//�������� �޸� �ּ�(�����ּ�)�� �� ����.
	//�ڵ� ���� ���μ������� ������ �� �ִ�.(�и� �ٸ� �������ӿ��� �ұ��ϰ�.) ������ �������� ���� �ּҴ� ���� �� ����.(���� �޸��� �ּ��̹Ƿ�)

	clock_t startTime;
	clock_t endTime;

	startTime = clock();

	//CriticalSection �ʱ�ȭ
	//InitializeCriticalSection(&cs);
	//Mutex
	WCHAR MutexName[] = L"�����Ǵ��ϱ�";
	MutexHandle = CreateMutex(NULL, FALSE, MutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("�̹� ���� ���Դϴ�..\n");
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

