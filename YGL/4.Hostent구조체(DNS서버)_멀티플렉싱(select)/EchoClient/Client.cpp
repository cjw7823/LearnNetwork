#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>//windows
#include <iostream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	cout << "<Client>" << endl;

	WSAData wsaData;

	SOCKET hServerSocket;

	SOCKADDR_IN serverAddr;

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
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//해당 IP주소는 무조건 내 랜카드의 주소다.
	serverAddr.sin_port = htons(12345);

	//3. connect
	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}

	while (1)
	{
		char message[1024] = "";
		printf("input : ");
		scanf("%s", message);

		int sendLength = send(hServerSocket, message, strlen(message), 0);

		int recvLength = recv(hServerSocket, message, sizeof(message), 0);
	
		cout << message << endl;
		


		Sleep(1000);
	}

	closesocket(hServerSocket);//클라이언트의 서버 소켓 닫기.

	WSACleanup();

	return 0;
}