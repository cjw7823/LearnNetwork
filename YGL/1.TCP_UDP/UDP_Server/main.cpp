#include <WinSock2.h>//windows
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	//_beginthread

	WSAData wsaData;

	SOCKET hServerSocket;

	SOCKADDR_IN serverAddr;//포트, 주소 등을 넣을 구조체
	SOCKADDR_IN clientAddr;

	//1.Winsock 초기화 전화기
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket 전화선
	hServerSocket = socket(PF_INET, SOCK_DGRAM, 0);
	//SOCK_STREAM연결지향=TCP / SOCK_DGRAM 택배마냥 한번에 보낸다.=UDP 보내는 과정 중에 섞이거나 손실 발생가능성.
	// PF_INET은 TCP/IP사용하기 때문에 고정. /SOCK_STREAM은 1,2번만 사용 1=TCP 2=UDP/ 프로토콜 타입 0
	if (hServerSocket == INVALID_SOCKET)//숫자(소켓넘버)를 만들었는데 숫자가 없다면 안만들어진 것. 
	{
		cout << "error socket" << endl;
		exit(-1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));//필요 없는 부분 0으로 채워줘야 하기 때문에 구조체 0으로 초기화.
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9190);


	//3. ip, port binding
	if (bind(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//다른 프로세스에서 포트를 이미 사용하고 있다면 에러가 난다.
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
		if (recvLength == -1)//서버와 접속 끊김.
		{
			cout << "error :" << hServerSocket << endl;
			exit(-1);
		}

		cout << "message from Client : " << message << endl;

		//6. send
		sendto(hServerSocket, message, recvLength, 0, (SOCKADDR*)&clientAddr, clientLength);
	}

	closesocket(hServerSocket);//서버 소켓 닫는다.

	WSACleanup();//1번에서 Startup한 걸 닫는다.

	return 0;
}