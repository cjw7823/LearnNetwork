#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>//windows
#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI Input(void* arg);

CRITICAL_SECTION cs;

string ClientID;

int main()
{
	WSAData wsaData;

	SOCKADDR_IN serverAddr;

	SOCKET hServerSocket;

	struct hostent* host;

	cout << "<Client>" << endl;

	cout << "����� ID�� �Է��ϼ���.\n" << endl;
	getline(cin, ClientID);
	cout << "--------------------" << endl;

	//1.Winsock �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//2.Create Socket
	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		cout << "error socket" << endl;
		exit(-1);
	}

	char domain[] = "work.junios.net";
	//printf("�������� �Է��ϼ���.\n");
	//cin >> domain;

	host = gethostbyname(domain);


	char a[1024] = { 0, };

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]));
	serverAddr.sin_port = htons(6666);

	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}

	HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, Input, (void*)&hServerSocket, 0, NULL);

	char Buffer[1024] = { 0, };

	while (1)
	{
		int RecvLength = recv(hServerSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)
		{
			break;
		}

		cout << Buffer << endl;
	}

	closesocket(hServerSocket); //Ŭ���̾�Ʈ�� ���� ���� �ݱ�.

	WSACleanup();

	return 0;
}

unsigned __stdcall Input(void* arg)
{
	string input = "";
	const char* Buffer;

	while (1)
	{
		getline(cin, input);

		input = ClientID + " : " + input;

		Buffer = input.c_str();

		SOCKET hServerSocket = *(SOCKET*)arg;
		send(hServerSocket, Buffer, strlen(Buffer)+1, 0);
	}

	return 0;
}
