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

	//1.Winsock �ʱ�ȭ
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
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//�ش� IP�ּҴ� ������ �� ��ī���� �ּҴ�.
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
	fp = fopen("2.png", "wb"); //���Ͽ���
	recv(hServerSocket, buf, sizeof(buf), 0); //���ϻ�����ޱ�

	file_size = atol(buf); //char->long��ȯ 
	totalBufferNum = file_size / sizeof(buf) + 1;
	//��ü������ = ������ü������ / �ް��ִµ�����

	while (BufferNum != totalBufferNum) {
		readBytes = recv(hServerSocket, buf, sizeof(buf), 0);
		//�����Ϳ� ��������ũ�� �ޱ� 
		BufferNum++;
		fwrite(buf, sizeof(char), readBytes, fp);
		//�����Ϳ� ��������ũ�⸸ŭ ���� 
	}

	fclose(fp);

	closesocket(hServerSocket);//Ŭ���̾�Ʈ�� ���� ���� �ݱ�.

	WSACleanup();

	return 0;
}