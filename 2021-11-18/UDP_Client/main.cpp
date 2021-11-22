#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>//windows
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	//_beginthread

	WSAData wsaData;

	SOCKET hServerSocket;

	SOCKADDR_IN serverAddr;//��Ʈ, �ּ� ���� ���� ����ü

	//1.Winsock �ʱ�ȭ ��ȭ��
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket ��ȭ��
	hServerSocket = socket(PF_INET, SOCK_DGRAM, 0);
	//SOCK_STREAM��������=TCP / SOCK_DGRAM �ù踶�� �ѹ��� ������.=UDP ������ ���� �߿� ���̰ų� �ս� �߻����ɼ�.
	// PF_INET�� TCP/IP����ϱ� ������ ����. /SOCK_STREAM�� 1,2���� ��� 1=TCP 2=UDP/ �������� Ÿ�� 0
	if (hServerSocket == INVALID_SOCKET)//����(���ϳѹ�)�� ������µ� ���ڰ� ���ٸ� �ȸ������ ��. 
	{
		cout << "error socket" << endl;
		exit(-1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));//�ʿ� ���� �κ� 0���� ä����� �ϱ� ������ ����ü 0���� �ʱ�ȭ.
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(9190);

	while (1)
	{
		//send
		char message[1024] = { 0, };
		cout << "������ ���� �޼��� : ";
		cin >> message;
		int recvLength = strlen(message);
		sendto(hServerSocket, message, recvLength + 1, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

		memset(message, 0, 1024);

		//4. recv recive
		int clientLength = sizeof(serverAddr);
		recvLength = recvfrom(hServerSocket, message, sizeof(message), 0, (SOCKADDR*)&serverAddr, &clientLength);
		if (recvLength == -1)//������ ���� ����.
		{
			cout << "error :" << hServerSocket << endl;
			exit(-1);
		}

		cout << "message from server : " << message << endl;
	}
	closesocket(hServerSocket);//���� ���� �ݴ´�.

	WSACleanup();//1������ Startup�� �� �ݴ´�.

	return 0;
}