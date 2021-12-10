#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>//windows
#include <iostream>
#include <windows.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI Input(void* arg);
unsigned WINAPI Output(void* arg);

CRITICAL_SECTION cs;

int main()
{
	cout << "<Client>" << endl;

	WSAData wsaData;

	SOCKET hServerSocket;

	SOCKADDR_IN serverAddr;

	HANDLE ThreadHandle[2];
	unsigned int threadID;

	InitializeCriticalSection(&cs);

	//1.Winsock 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket
	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		cout << "error socket" << endl;
		exit(-1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("172.16.2.254");//해당 IP주소는 무조건 내 랜카드의 주소다.
	serverAddr.sin_port = htons(12345);

	//3. connect
	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}

		ThreadHandle[0] = (HANDLE)_beginthreadex(NULL, 0, Input, (void*)&hServerSocket, 0, &threadID);
		ThreadHandle[1] = (HANDLE)_beginthreadex(NULL, 0, Output, (void*)&hServerSocket, 0, &threadID);
	

	WaitForMultipleObjects(2, ThreadHandle, TRUE, INFINITE);

	DeleteCriticalSection(&cs);

	closesocket(hServerSocket); //클라이언트의 서버 소켓 닫기.

	WSACleanup();

	return 0;
}

unsigned __stdcall Input(void* arg)
{
	while (1)
	{
		char message[1024] = "";
		cin >> message;

		send(*(SOCKET*)arg, message, strlen(message), 0);

	}
	
	return 0;
}

unsigned __stdcall Output(void* arg)
{
	while (1)
	{
		EnterCriticalSection(&cs);

		char message2[1024] = "";
		recv(*(SOCKET*)arg, message2, sizeof(message2), 0);

		LeaveCriticalSection(&cs);

		cout << message2 << endl;

	}
	
	return 0;
}
