#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>//windows
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSAData wsaData;

	SOCKET hServerSocket;

	SOCKADDR_IN serverAddr;

	//1.Winsock √ ±‚»≠
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
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(9190);

	//3. connect
	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}


	//4. recv recive
	char message[1024] = { 0, };
	int recvLength = recv(hServerSocket, message, sizeof(message), 0);
	if (recvLength == -1)
	{
		cout << "error recv" << endl;
		exit(-1);
	}

	cout << message << endl;

	closesocket(hServerSocket);

	WSACleanup();

	return 0;
}