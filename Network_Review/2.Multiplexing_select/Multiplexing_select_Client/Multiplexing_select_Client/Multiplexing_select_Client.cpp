#include <WinSock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadFunction(_In_ LPVOID param)
{
	std::cout << "___Begin Thread___" << std::endl;

	SOCKET soc = *(SOCKET*)(param);
	char buffer[100] = { 0, };
	while (recv(soc, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
	{		
		std::cout << buffer << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}

	std::cout << "___END Thread___" << std::endl;

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

	if (connect(h_ClientSocket, (sockaddr*)&addr, sizeof(addr)))
	{
		std::cout << "connect() Failed" << std::endl;
		closesocket(h_ClientSocket);
		WSACleanup();
		return 0;
	}

	/*
		서버와 연결 성공 시, 메인 스레드에서 send / 서브 스레드에서 recv
	*/
	DWORD dw_ThreadID;
	CreateThread(NULL,
		0,
		ThreadFunction,
		(LPVOID)&h_ClientSocket,
		0,
		&dw_ThreadID);

	while (1)
	{
		std::string str;
		std::cin >> str;
		if (str == "EXIT")
			break;

		send(h_ClientSocket, str.c_str(), (int)str.length(), 0);
	}
	closesocket(h_ClientSocket);//소켓을 닫아 서브 스레드의 recv 블록킹 해제.

	/*
		서브 스레드 종료를 체크.
	*/
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dw_ThreadID);
	if (hThread != NULL)
	{
		DWORD exitCode;
		while (1)
		{
			if (GetExitCodeThread(hThread, &exitCode))
			{
				if (exitCode == STILL_ACTIVE)
					Sleep(100);
				else
					break;
			}
		}
		CloseHandle(hThread);
	}

	WSACleanup();

	return 0;
}