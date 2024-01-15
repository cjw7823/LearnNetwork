#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main()
{
	FILE* sourceFile = nullptr;

	SOCKET ServerSocket;

	SOCKADDR_IN ServerAddr;

	WSAData wsaData;

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup");
		exit(-1);
	}

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("socket");
		exit(-1);
	}

	memset(&ServerAddr, 0, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = PF_INET; //IP V4
	ServerAddr.sin_port = htons(12345);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //아무거나

	//3. connect
	if (connect(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}

	sourceFile = fopen("1.png", "rb");

	char Buffer[1024] = { 0, };
	int ReadLength = 0;
	char message[1024] = "";

	while ((ReadLength = fread(Buffer, sizeof(char), sizeof(Buffer), sourceFile)) != 0)
	{
		int SendLength = send(ServerSocket, Buffer, ReadLength, 0);
		printf("send : %d", SendLength);
	}

	shutdown(ServerSocket, SD_SEND);

	recv(ServerSocket, Buffer, sizeof(Buffer), 0);

	printf("from client : %s", Buffer);

	fclose(sourceFile);

	closesocket(ServerSocket);

	printf("disconnect\n");

	fclose(sourceFile);

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}