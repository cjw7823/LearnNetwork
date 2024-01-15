#include <WinSock2.h>//windows
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	cout << "<Server>" << endl;
	//_beginthread

	WSAData wsaData;

	SOCKET hServerSocket;
	SOCKET hClientSocket;

	SOCKADDR_IN serverAddr;//��Ʈ, �ּ� ���� ���� ����ü
	SOCKADDR_IN clientAddr;

	//1.Winsock �ʱ�ȭ ��ȭ��
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket ��ȭ��
	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);//SOCK_STREAM�������� / SOCK_DGRAM �ù踶�� �ѹ��� ������. ������ ���� �߿� ���̰ų� �ս� �߻����ɼ�.
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

	//4. ���
	if (listen(hServerSocket, 0) == SOCKET_ERROR)// 0�� ��� �ð� �ִ� ��. ������ �ð��� ����.
	{
		cout << "error listen" << endl;
		exit(-1);
	}

	
	//5. accept
	int clientAddrSize = sizeof(clientAddr);
	hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
	//���� ������ �Է��� ������ Ŭ���̾�Ʈ ������ ����� �ѱ�� ����. ������ 10���� Ŭ�� ������ Ŭ�� ������ 10���� ���������.
	if (hClientSocket == SOCKET_ERROR)
	{
		cout << "error accept" << endl;
		exit(-1);
	}

	while (1)
	{
		//4. recv recive
		char message[1024] = { 0, };
		int recvLength = recv(hClientSocket, message, sizeof(message), 0);
		if (recvLength == -1)//������ ���� ����.
		{
			cout << "Client Disconnect :" << hClientSocket << endl;
			closesocket(hClientSocket);//new ������ Delete�ߵ�. ���� ����

			//accept
			int clientAddrSize = sizeof(clientAddr);
			hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
			//���� ������ �Է��� ������ Ŭ���̾�Ʈ ������ ����� �ѱ�� ����. ������ 10���� Ŭ�� ������ Ŭ�� ������ 10���� ���������.
			if (hClientSocket == SOCKET_ERROR)
			{
				cout << "error accept" << endl;
				exit(-1);
			}
			

			//exit(-1);
		}

		cout << message << endl;



		//6. send
		send(hClientSocket, message, recvLength, 0);//0�� �׳� �ܿ���. ���ڿ��� nullptr������ +1���ش�.
	}

	closesocket(hClientSocket);//5������ ���� ������ ���� ���� Ŭ�� ������ �ݴ´�.
	closesocket(hServerSocket);//���� ���� �ݴ´�.

	WSACleanup();//1������ Startup�� �� �ݴ´�.

	return 0;
}