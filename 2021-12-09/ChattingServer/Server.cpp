#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI ServerSocketAccept(void* arg);
unsigned WINAPI Broadcasting(void* arg);

CRITICAL_SECTION cs;

struct SocketData
{

	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddr;
	SOCKADDR_IN ClientAddr;

};

vector<SOCKET> vClientSocket;

int main()
{
	SOCKET ClientSocket;

	SocketData SocketData;

	WSAData wsaData;

	HANDLE ThreadHandle[20];
	unsigned int threadID;
	InitializeCriticalSection(&cs);

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup");
		exit(-1);
	}

	SocketData.ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketData.ServerSocket == INVALID_SOCKET)
	{
		printf("socket");
		exit(-1);
	}

	memset(&SocketData.ServerAddr, 0, sizeof(SOCKADDR_IN));
	SocketData.ServerAddr.sin_family = PF_INET;
	SocketData.ServerAddr.sin_port = htons(12345);
	SocketData.ServerAddr.sin_addr.s_addr = INADDR_ANY;

	bind(SocketData.ServerSocket, (SOCKADDR*)&SocketData.ServerAddr, sizeof(SocketData.ServerAddr));

	listen(SocketData.ServerSocket, 5);

	printf("Server\n");

	while (1)
	{
		int clientAddrSize = sizeof(SocketData.ClientAddr);
		EnterCriticalSection(&cs);
		vClientSocket.push_back(ClientSocket=accept(SocketData.ServerSocket, (SOCKADDR*)&SocketData.ClientAddr, &clientAddrSize));
		if (vClientSocket.back() == SOCKET_ERROR)
		{
			cout << "error accept" << endl;
			exit(-1);
		}
		else
		{
			ThreadHandle[(vClientSocket.size()) - 1] = (HANDLE)_beginthreadex(NULL, 0, Broadcasting, (void*)&ClientSocket, 0, &threadID);
		}
		LeaveCriticalSection(&cs);
	}

	WaitForMultipleObjects(20, ThreadHandle, TRUE, INFINITE);
	DeleteCriticalSection(&cs);

	closesocket(SocketData.ServerSocket);

	WSACleanup();

	return 0;
}

unsigned __stdcall Broadcasting(void* arg)
{
	while (1)
	{
		char message[1024] = { 0, };
		if ( ( recv( *(SOCKET*)arg, message, sizeof(message), 0)) == 0)
		{
			closesocket(*(SOCKET*)arg);
			cout << "CloseSocket" << endl;
		}
		else
		{
			cout << message << endl;
			for (unsigned int i = 0; i < vClientSocket.size(); i++)
			{
					send(vClientSocket.at(i), message, strlen(message) + 1, 0);
			}
		}

	}

	return 0;
}
