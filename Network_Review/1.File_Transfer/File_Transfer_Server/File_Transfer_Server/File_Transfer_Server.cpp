#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
	std::cout << "<Server>" << std::endl;

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
	SOCKET h_ServerSocket;
	SOCKET h_ClientSocket;
	h_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (h_ServerSocket == INVALID_SOCKET)//소켓 생성 실패 시.
	{
		std::cout << "socket() Failed" << std::endl;
		WSACleanup();
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1");
	if (bind(h_ServerSocket, (sockaddr*)(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "bind() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	if (listen(h_ServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listen() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	sockaddr addrClient;
	int len = sizeof(addrClient);
	h_ClientSocket = accept(h_ServerSocket, &addrClient, &len);
	if (h_ClientSocket == INVALID_SOCKET)
	{
		std::cout << "accept() Failed" << std::endl;
		closesocket(h_ServerSocket);
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	/*
		파일 전송.
		파일 사이즈 전송 후, 파일 전송.
		파일 사이즈는 무조건 도작한다고 가정.
	*/
	std::cout << "파일 전송을 시작합니다." << std::endl;

	FILE* fp;
	if (fopen_s(&fp, "1.jpg", "rb")) //파일열기
	{
		std::cout << "fopen() Failed" << std::endl;
		closesocket(h_ServerSocket);
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	int sendBytes;
	long file_size;
	char buffer[1024] = { 0, };
	fseek(fp, 0, SEEK_END); //끝으로가서
	file_size = ftell(fp); //사이즈 측정
	fseek(fp, 0, SEEK_SET); //처음으로와서 
	snprintf(buffer, sizeof(buffer), "%d", file_size); //사이즈값을 buffer에 넣기 
	std::cout << "File Size : " << buffer << std::endl;

	send(h_ClientSocket, buffer, sizeof(buffer), 0); //사이즈값전송 
	memset(buffer, 0, sizeof(buffer));//버퍼 초기화

	while ((sendBytes = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
	{
		send(h_ClientSocket, buffer, sendBytes, 0);
	}

	std::cout << "파일 전송 완료." << std::endl;
	send(h_ClientSocket, "Done", 5, 0);

	fclose(fp);

	closesocket(h_ServerSocket);
	closesocket(h_ClientSocket);
	WSACleanup();

	return 0;
}