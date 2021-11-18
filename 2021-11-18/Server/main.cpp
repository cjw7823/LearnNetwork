#include <WinSock2.h>//windows
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSAData wsaData;

	SOCKET hServerSocket;
	SOCKET hClientSocket;

	SOCKADDR_IN serverAddr;
	SOCKADDR_IN clientAddr;

	//1.Winsock 초기화 전화기
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket 전화선
	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		cout << "error socket" << endl;
		exit(-1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9190);


	//3. ip, port binding
	if (bind(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error bind" << endl;
		exit(-1);
	}

	//4. 대기
	if (listen(hServerSocket, 0) == SOCKET_ERROR)
	{
		cout << "error listen" << endl;
		exit(-1);
	}

	//5. accept
	int clientAddrSize = sizeof(clientAddr);
	hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
	if (hClientSocket == SOCKET_ERROR)
	{
		cout << "error accept" << endl;
		exit(-1);
	}

	//6. send
	char message[] = "Hello World";
	send(hClientSocket, message, sizeof(message), 0);

	closesocket(hClientSocket);
	closesocket(hServerSocket);

	WSACleanup();

	return 0;
}