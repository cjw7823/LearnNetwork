#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <list>

#pragma comment(lib, "ws2_32.lib")

/* 전역 변수 */
CRITICAL_SECTION g_cs;
std::list<SOCKET> g_socketList;	// 해당 리스트는 CRITICAL_SECTION으로 원자성을 보장하도록 한다.
SOCKET g_serverSocket;

BOOL CtrlHandler(DWORD dwType)
{
	if (dwType == CTRL_C_EVENT)
	{
		closesocket(g_serverSocket);

		EnterCriticalSection(&g_cs);

		LeaveCriticalSection(&g_cs);

		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI ThreadFunction(LPVOID param)
{
	SOCKET h_clientSocket = (SOCKET)param;
	std::cout << "클라이언트 접속 : " << h_clientSocket << std::endl;

	char buffer[1024] = { 0, };
	while (recv(h_clientSocket, buffer, sizeof(buffer), 0) >= 0)
	{
		//모든 클라이언트에 메세지 전송
		int len = strlen(buffer);
		EnterCriticalSection(&g_cs);
		for (auto it = g_socketList.begin(); it != g_socketList.end(); it++)
		{
			if(*it != h_clientSocket)//메세지를 전송한 클라이언트는 제외.
				send(*it, buffer, sizeof(char) * (len + 1), 0);
		}
		LeaveCriticalSection(&g_cs);

		memset(buffer, 0, sizeof(buffer));
	}

	EnterCriticalSection(&g_cs);
	g_socketList.remove(h_clientSocket);
	LeaveCriticalSection(&g_cs);
	std::cout << "클라이언트 접속 해제 : " << h_clientSocket << std::endl;
	return 0;
}

int main(int argc, char* argv[])
{
	std::cout << "<MultiThread Chatting Server>" << std::endl;
	std::cout << "Ctrl + C 를 누르면 서버를 종료합니다." << std::endl;

	WSADATA wsaData = { 0, };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		WSACleanup();
		return 0;
	}

	SOCKET h_ServerSocket;
	SOCKET h_ClientSocket;
	h_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (h_ServerSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	sockaddr_in addrServer;
	sockaddr_in addrClient;
	int addr_len = sizeof(addrClient);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr);
	if (bind(h_ServerSocket, (sockaddr*)&addrServer, sizeof(addrServer)))
	{
		std::cout << "bind() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}
	if (listen(h_ServerSocket, SOMAXCONN))
	{
		std::cout << "listen() Failed" << std::endl;
		closesocket(h_ServerSocket);
		WSACleanup();
		return 0;
	}

	//임계영역 객체 생성.
	InitializeCriticalSection(&g_cs);

	//서버종료 이벤트 등록.
	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
		puts("SetConsoleCtrlHandler() Failed");

	while ((h_ClientSocket = accept(h_ServerSocket, (sockaddr*)&addrClient, &addr_len)) != INVALID_SOCKET)
	{
		EnterCriticalSection(&g_cs);
		g_socketList.push_back(h_ClientSocket);
		LeaveCriticalSection(&g_cs);

		CreateThread(
			NULL,					//보안 속성
			0,						//스택 메모리 크기
			ThreadFunction,			//함수
			(LPVOID)h_ClientSocket,	//매개 변수
			0,						//생성 플레그
			NULL					//스레드 ID
		);
	}

	closesocket(h_ServerSocket);
	closesocket(h_ClientSocket);
	WSACleanup();

	return 0;
}