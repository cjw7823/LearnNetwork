#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


int main(int args, char* argv[])
{
	std::cout << "<Client>" << std::endl;

	/*
		윈도우 소켓 라이브러리 초기화.
	*/
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))//성공하면 0 반환.
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		return 0;
	}

	/*
		Socket 설정.
	*/
	SOCKET h_Socket;
	h_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (h_Socket == INVALID_SOCKET)//소켓 생성 실패 시.
	{
		std::cout << "socket() Failed" << std::endl;
		WSACleanup();
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (connect(h_Socket, (sockaddr*)&addr, sizeof(addr)))
	{
		std::cout << "connect() Failed" << std::endl;
		closesocket(h_Socket);
		WSACleanup();
		return 0;
	}

	std::cout << "파일 수신을 시작합니다." << std::endl;

	char buffer[1024] = { 0, };
	recv(h_Socket, buffer, sizeof(buffer), 0);
	int maxFileSize = atol(buffer);
	printf("File Size : %d \n", maxFileSize);

	FILE* fp={0,};
	fopen_s(&fp, "2.jpg", "wb");

	int recvSize = 0;
	int curSize = 0;
	while (curSize != maxFileSize)
	{
		if (maxFileSize - curSize >= sizeof(buffer))
			recvSize = recv(h_Socket, buffer, sizeof(buffer), 0);
		else
			recvSize = recv(h_Socket, buffer, maxFileSize - curSize, 0);

		curSize += recvSize;
		fwrite(buffer, sizeof(char), recvSize, fp);
	}

	std::cout << "파일 수신 완료." << std::endl;
	recv(h_Socket, buffer, sizeof(buffer), 0);

	std::cout << buffer << std::endl;

	return 0;
}