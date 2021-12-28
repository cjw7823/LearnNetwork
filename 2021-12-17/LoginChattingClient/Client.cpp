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

string NickName;

int main()
{
	WSAData wsaData;

	SOCKADDR_IN serverAddr;

	SOCKET hServerSocket;

	cout << "<Client>" << endl;

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

	char a[1024] = { 0, };

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(12345);

	if (connect(hServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "error connect" << endl;
		exit(-1);
	}


	//���⼭ �α���??
	while (1)
	{
		char userID[20] = { 0, };
		char userPW[20] = { 0, };
		char checklogin[20] = { 0, };

		cout << "�α��� ID �Է� : ";
		cin >> userID;
		cout << "�α��� PW �Է� : ";
		cin >> userPW;

		send(hServerSocket, userID, strlen(userID) + 1, 0);
		send(hServerSocket, userPW, strlen(userPW) + 1, 0);

		cout << "�α��� Ȯ����..." << endl;

		int RecvLength = recv(hServerSocket, checklogin, sizeof(checklogin), 0);
		if (RecvLength <= 0)//������ ����� ����
		{
			cout << "���� ����" << endl;
			exit(-1);
		}
		else if (checklogin == "����");
		{
			cout << "�α��� ����" << endl;
			break;
		}
	}


	cout << "����� NickName�� �Է��ϼ���.\n" << endl;
	getline(cin, NickName);
	cout << "--------------------" << endl;

	HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, Input, (void*)&hServerSocket, 0, NULL);

	char Buffer[1024] = { 0, };

	while (1)
	{
		int RecvLength = recv(hServerSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)//������ ����� ����
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
	string input="";
	const char* Buffer;

	while (1)
	{
		getline(cin, input);

		input = NickName + " : " + input;

		Buffer = input.c_str();

		SOCKET hServerSocket = *(SOCKET*)arg;
		send(hServerSocket, Buffer, strlen(Buffer) + 1, 0);
	}

	return 0;
}
