#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <list>

#pragma comment(lib, "ws2_32.lib")

/* ���� ���� */
CRITICAL_SECTION g_cs;
std::list<SOCKET> g_socketList;	// �ش� ����Ʈ�� CRITICAL_SECTION���� ���ڼ��� �����ϵ��� �Ѵ�.
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
	std::cout << "Ŭ���̾�Ʈ ���� : " << h_clientSocket << std::endl;

	char buffer[1024] = { 0, };
	while (recv(h_clientSocket, buffer, sizeof(buffer), 0) >= 0)
	{
		//��� Ŭ���̾�Ʈ�� �޼��� ����
		int len = strlen(buffer);
		EnterCriticalSection(&g_cs);
		for (auto it = g_socketList.begin(); it != g_socketList.end(); it++)
		{
			if(*it != h_clientSocket)//�޼����� ������ Ŭ���̾�Ʈ�� ����.
				send(*it, buffer, sizeof(char) * (len + 1), 0);
		}
		LeaveCriticalSection(&g_cs);

		memset(buffer, 0, sizeof(buffer));
	}

	EnterCriticalSection(&g_cs);
	g_socketList.remove(h_clientSocket);
	LeaveCriticalSection(&g_cs);
	std::cout << "Ŭ���̾�Ʈ ���� ���� : " << h_clientSocket << std::endl;
	return 0;
}

int main(int argc, char* argv[])
{
	std::cout << "<MultiThread Chatting Server>" << std::endl;
	std::cout << "Ctrl + C �� ������ ������ �����մϴ�." << std::endl;

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

	//�Ӱ迵�� ��ü ����.
	InitializeCriticalSection(&g_cs);

	//�������� �̺�Ʈ ���.
	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
		puts("SetConsoleCtrlHandler() Failed");

	while ((h_ClientSocket = accept(h_ServerSocket, (sockaddr*)&addrClient, &addr_len)) != INVALID_SOCKET)
	{
		EnterCriticalSection(&g_cs);
		g_socketList.push_back(h_ClientSocket);
		LeaveCriticalSection(&g_cs);

		CreateThread(
			NULL,					//���� �Ӽ�
			0,						//���� �޸� ũ��
			ThreadFunction,			//�Լ�
			(LPVOID)h_ClientSocket,	//�Ű� ����
			0,						//���� �÷���
			NULL					//������ ID
		);
	}

	closesocket(h_ServerSocket);
	closesocket(h_ClientSocket);
	WSACleanup();

	return 0;
}