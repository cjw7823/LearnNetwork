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
	SOCKADDR_IN clientAddr;

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
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9190);


	//3. ip, port binding
	if (bind(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//�ٸ� ���μ������� ��Ʈ�� �̹� ����ϰ� �ִٸ� ������ ����.
	{
		cout << "error bind" << endl;
		exit(-1);
	}

	while (1)
	{
		//4. recv recive
		char message[1024] = { 0, };
		int clientLength = sizeof(clientAddr);
		int recvLength = recvfrom(hServerSocket, message, sizeof(message), 0, (SOCKADDR*)&clientAddr, &clientLength);
		if (recvLength == -1)//������ ���� ����.
		{
			cout << "error :" << hServerSocket << endl;
			exit(-1);
		}

		cout << "message from Client : " << message << endl;

		//6. send
		sendto(hServerSocket, message, recvLength, 0, (SOCKADDR*)&clientAddr, clientLength);
	}

	closesocket(hServerSocket);//���� ���� �ݴ´�.

	WSACleanup();//1������ Startup�� �� �ݴ´�.

	return 0;
}