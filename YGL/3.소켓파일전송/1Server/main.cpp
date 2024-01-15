#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	FILE* sourceFile = nullptr;

	SOCKET ServerSocket;
	SOCKET ClientSocket;

	SOCKADDR_IN ServerAddr;
	SOCKADDR_IN ClientAddr;

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
	ServerAddr.sin_addr.s_addr = INADDR_ANY; //아무거나

	bind(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	listen(ServerSocket, 5);

		int ClientAddrLength = sizeof(ClientAddr);
		ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddr, &ClientAddrLength);

		sourceFile = fopen("2.png", "wb");

		char Buffer[1024] = { 0, };
		int ReadLength = 0;
		char message[1024] = "";
		while ((ReadLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0)) != 0)
		{
			fwrite(Buffer, 1, ReadLength, sourceFile);
		}

		char message2[] = "Done";
		send(ClientSocket, message2, sizeof(message2), 0);

		fclose(sourceFile);

		closesocket(ClientSocket);

		printf("disconnect\n");
	
	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}