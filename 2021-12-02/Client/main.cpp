#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

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

	int totalBufferNum;
	int BufferNum = 0;
	int readBytes;
	long file_size;
	char buf[1024]="";

	FILE* fp;
	fp = fopen("2.png", "wb"); //파일열고
	recv(hServerSocket, buf, sizeof(buf), 0); //파일사이즈받기

	file_size = atol(buf); //char->long변환 
	totalBufferNum = file_size / sizeof(buf) + 1;
	//전체사이즈 = 파일전체사이즈 / 받고있는데이터

	while (BufferNum != totalBufferNum) {
		readBytes = recv(hServerSocket, buf, sizeof(buf), 0);
		//데이터와 데이터의크기 받기 
		BufferNum++;
		fwrite(buf, sizeof(char), readBytes, fp);
		//데이터와 데이터의크기만큼 쓰기 
	}

	fclose(fp);

	closesocket(hServerSocket);//클라이언트의 서버 소켓 닫기.

	WSACleanup();

	return 0;
}