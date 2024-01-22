#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadFunction(LPVOID param)
{
	SOCKET h_socket = (SOCKET)param;

	//���� send
	while (1)
	{
		std::string str;
		getline(std::cin, str);
		if (str == "EXIT")
			break;
		send(h_socket, str.c_str(), (int)str.length() + 1, 0);
	}

	//������ �ݾ� ���� �������� recv ��� ����.
	closesocket(h_socket);

	return 0;
}

int main()
{
	std::cout << "<Login Chatting Client>" << std::endl;

	/*
		���� ����. �������� ���� ����ó�� ����.
	*/
	WSADATA wsaData = { 0, };
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET h_Socket;
	h_Socket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	if (connect(h_Socket, (sockaddr*)&addr, sizeof(addr)))
		return 0;

	/*
		�α��� ���� ��, ������ ����.
	*/
	HANDLE h_thread = 0;
	bool b_flagLogin = false;
	while (!b_flagLogin)
	{
		system("cls");
		std::cout << "<Login Chatting Client>" << std::endl;

		char buffer[1024] = { 0, };
		std::string id;
		std::string pw;

		std::cout << "�α����� �õ��մϴ�.\n ���̵� : ";
		std::getline(std::cin, id);

		std::cout << " ��й�ȣ : ";
		std::getline(std::cin, pw);
		//�������� ����.
		if (pw.empty() ||
			!std::all_of(pw.begin(),
				pw.end(),
				[](char c) {	return std::isdigit(static_cast<unsigned char>(c)); }))
		{
			continue;
		}

		::send(h_Socket, (id + "/" + pw).c_str(), (int)(id + "/" + pw).length() + 1, 0);//id / pw �۽�.
		::recv(h_Socket, buffer, sizeof(buffer), 0);//�α��� ��� ����.

		std::string str = buffer;
		if (str == "Login") //�α��� ����.
		{
			b_flagLogin = true;

			//������ ����
			h_thread = CreateThread(
				NULL,
				0,
				ThreadFunction,
				(LPVOID)h_Socket,
				0,
				NULL
			);

			//�α��� ���� â ����.
			system("cls");
			std::cout << "<Login Chatting Client>" << std::endl;
			std::cout << "�α��� ����. ä�� ����." << std::endl;

			memset(buffer, 0, sizeof(buffer));
			//���� recv
			while (recv(h_Socket, buffer, sizeof(buffer), 0) >= 0)
			{
				std::cout << " ->" << buffer << std::endl;
				memset(buffer, 0, sizeof(buffer));
			}
		}
	}

	/*
		���� ������ ���� Ȯ��.
	*/
	DWORD exitCode;
	while (1)
	{
		if (GetExitCodeThread(h_thread, &exitCode))
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