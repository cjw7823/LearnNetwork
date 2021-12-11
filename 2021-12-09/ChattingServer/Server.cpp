#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI Broadcasting(void* arg);

CRITICAL_SECTION cs;

vector<SOCKET> vClientSocket;

int main()
{
	cout << "Server" << endl;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerADDR;

	SOCKET ClientSocket;
	SOCKADDR_IN ClientADDR;

	InitializeCriticalSection(&cs);

	WSAData WsaData;

	//Initailize Winsock 
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//서버 주소 구조체 초기화
	memset(&ServerADDR, 0, sizeof(ServerADDR));
	ServerADDR.sin_family = AF_INET;
	ServerADDR.sin_addr.s_addr = INADDR_ANY;
	ServerADDR.sin_port = htons(12345);

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

	bind(ServerSocket, (SOCKADDR*)&ServerADDR, sizeof(ServerADDR));

	listen(ServerSocket, 5);

	while (true)
	{
		int ClientAddrSize = sizeof(ClientADDR);
		ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientADDR, &ClientAddrSize);
		cout << "Connect : " << ClientSocket << endl;
		//ClientSocket을 기록
		EnterCriticalSection(&cs);
		vClientSocket.push_back(ClientSocket);
		LeaveCriticalSection(&cs);
		//worker thread
		HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, Broadcasting, (void*)&ClientSocket, 0, NULL);
		//ClientSocket 데이터 처리, recv, send(Thread)
	}

	closesocket(ServerSocket);

	DeleteCriticalSection(&cs);

	WSACleanup();

	return 0;
}

unsigned __stdcall Broadcasting(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;
	char Buffer[1024] = { 0, };

	while (true)
	{
		int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)
		{
			//연결이 끊겼을때
			closesocket(ClientSocket);
			EnterCriticalSection(&cs);
			for (auto iter = vClientSocket.begin(); iter != vClientSocket.end(); ++iter)
			{
				if (*iter == ClientSocket)
				{
					vClientSocket.erase(iter);
					cout << "CloseSocket : " << ClientSocket << endl;
					break;
				}
			}
			LeaveCriticalSection(&cs);
		}
		else
		{
			EnterCriticalSection(&cs);
			for (auto iter = vClientSocket.begin(); iter != vClientSocket.end(); ++iter)
			{
				send(*iter, Buffer, strlen(Buffer)+1, 0);
			}
			LeaveCriticalSection(&cs);
		}
	}

	return 0;
}
