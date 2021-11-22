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
	serverAddr.sin_port = htons(9190);

	//3. connect
	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}

	while(1)
	{
		char message[1024] = { 0, };
		cout << "서버로 보낼 내용 :";
		cin >> message;

		send(hServerSocket, message, strlen(message) + 1, 0);


	
		/*char message2[] = "\r"; //개행 문자를 요청 신호로 받아들인다.
		send(hServerSocket, message2, sizeof(message2), 0);*/


		//4. recv recive
		int recvLength = recv(hServerSocket, message, sizeof(message), 0);
		if (recvLength == -1)//서버와 접속 끊김. 
		{
			cout << "error recv" << endl;
			exit(-1);
		}

		cout << message << endl;
	}

	closesocket(hServerSocket);//클라이언트의 서버 소켓 닫기.

	WSACleanup();

	return 0;
}