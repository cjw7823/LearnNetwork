#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadFunction(_In_ LPVOID param)
{
	std::cout << "ThreadFunction" << std::endl;

	return 0;
}

int main(int argc, char* argv[])
{
	std::cout << "<Multi Thread Client>" << std::endl;

	WSADATA wsaData = { 0, };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		return 0;
	}

	SOCKET h_ClientSocket;
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

	if (h_ClientSocket = connect(h_ClientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		std::cout << "connect() Failed" << std::endl;
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	CreateThread(NULL,
		0,
		ThreadFunction,
		(LPVOID)h_ClientSocket,
		0,
		NULL);


	closesocket(h_ClientSocket);
	WSACleanup();

	return 0;
}