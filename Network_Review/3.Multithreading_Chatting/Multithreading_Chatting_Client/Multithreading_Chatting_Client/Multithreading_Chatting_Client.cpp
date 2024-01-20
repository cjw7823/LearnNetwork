#include <WinSock2.h>
#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadFunction(LPVOID param)
{
	SOCKET socket = (SOCKET)param;

	//무한 send
	while (1)
	{
		
		std::string str;
		getline(std::cin, str);
		if (str == "EXIT")
			break;
		send(socket, str.c_str(), str.length() + 1, 0);
	}

	//소켓을 닫아 메인 스레드의 recv 블록 해제.
	closesocket(socket);

	return 0;
}

int main(int argc, char* argv[])
{
	std::cout << "<MultiThread Client>" << std::endl;

	WSADATA wsaData = { 0, };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		WSACleanup();
		return 0;
	}

	SOCKET h_ClientSocket = { 0, };
	h_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (h_ClientSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	if (connect(h_ClientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		std::cout << "connect() Failed" << std::endl;
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	//sender 스레드 생성
	HANDLE result = CreateThread(
		NULL,					//보안 속성
		0,						//스택 사이즈
		ThreadFunction,			//함수
		(LPVOID)h_ClientSocket,	//매개변수
		0,						//생성 플래그
		NULL);					//스레드 ID
	if (result == NULL)
	{
		std::cout << "CreateThread() Failed" << std::endl;
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	//무한 recv
	while (1)
	{
		char buffer[1024] = { 0, };
		if (recv(h_ClientSocket, buffer, sizeof(buffer), 0) <= 0)
		{
			std::cout << "서버가 종료되었습니다." << std::endl;
			break;
		}
		std::cout << buffer << std::endl;
	}

	/*
		서브 스레드 종료 확인.
	*/
	DWORD exitCode;
	while (1)
	{
		if (GetExitCodeThread(result, &exitCode))
		{
			if (exitCode == STILL_ACTIVE)
				Sleep(100);
			else
				break;
		}
	}

	WSACleanup();

	return 0;
}