#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include <WinSock2.h>//windows

#pragma comment(lib, "ws2_32.lib")

using namespace std;



int main()
{
	cout << "<Server>" << endl;
	//_beginthread

	WSAData wsaData;

	SOCKET hServerSocket;
	SOCKET hClientSocket;

	SOCKADDR_IN serverAddr;//포트, 주소 등을 넣을 구조체
	SOCKADDR_IN clientAddr;

	//1.Winsock 초기화 전화기
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket 전화선
	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);//SOCK_STREAM연결지향 / SOCK_DGRAM 택배마냥 한번에 보낸다. 보내는 과정 중에 섞이거나 손실 발생가능성.
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

	//4. 대기
	if (listen(hServerSocket, 0) == SOCKET_ERROR)// 0은 대기 시간 넣는 곳. 정해진 시간이 없다.
	{
		cout << "error listen" << endl;
		exit(-1);
	}


	//5. accept
	int clientAddrSize = sizeof(clientAddr);
	hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
	//서버 소켓은 입력이 들어오면 클라이언트 소켓을 만들어 넘기는 역할. 서버에 10개의 클라가 들어오면 클라 소켓이 10개가 만들어진다.
	if (hClientSocket == SOCKET_ERROR)
	{
		cout << "error accept" << endl;
		exit(-1);
	}

	int sendBytes;
	long file_size;
	char buf[1024] = "";

	FILE* fp;
	fp = fopen("1.png", "rb"); //파일열기
	fseek(fp, 0, SEEK_END); //끝으로가서 
	file_size = ftell(fp); //사이즈재고 
	cout << file_size<< endl;
	fseek(fp, 0, SEEK_SET); //처음으로와서 
	snprintf(buf, sizeof(buf), "%d", file_size); //사이즈값을 buf에다가넣기 
	cout <<buf<< endl;
	send(hClientSocket, buf, sizeof(buf), 0); //사이즈값전송 

	while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		send(hClientSocket, buf, sendBytes, 0);
	}

	fclose(fp);

	closesocket(hServerSocket);//서버 소켓 닫는다.
	closesocket(hClientSocket);

	WSACleanup();//1번에서 Startup한 걸 닫는다.


}