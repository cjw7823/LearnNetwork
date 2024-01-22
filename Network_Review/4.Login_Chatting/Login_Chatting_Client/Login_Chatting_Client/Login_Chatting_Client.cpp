#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadFunction(LPVOID param)
{
	SOCKET h_socket = (SOCKET)param;

	//무한 send
	while (1)
	{
		std::string str;
		getline(std::cin, str);
		if (str == "EXIT")
			break;
		send(h_socket, str.c_str(), (int)str.length() + 1, 0);
	}

	//소켓을 닫아 메인 스레드의 recv 블록 해제.
	closesocket(h_socket);

	return 0;
}

int main()
{
	std::cout << "<Login Chatting Client>" << std::endl;

	/*
		소켓 설정. 가독성을 위해 예외처리 생략.
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
		로그인 성공 시, 스레드 생성.
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

		std::cout << "로그인을 시도합니다.\n 아이디 : ";
		std::getline(std::cin, id);

		std::cout << " 비밀번호 : ";
		std::getline(std::cin, pw);
		//숫자인지 검증.
		if (pw.empty() ||
			!std::all_of(pw.begin(),
				pw.end(),
				[](char c) {	return std::isdigit(static_cast<unsigned char>(c)); }))
		{
			continue;
		}

		::send(h_Socket, (id + "/" + pw).c_str(), (int)(id + "/" + pw).length() + 1, 0);//id / pw 송신.
		::recv(h_Socket, buffer, sizeof(buffer), 0);//로그인 결과 수신.

		std::string str = buffer;
		if (str == "Login") //로그인 성공.
		{
			b_flagLogin = true;

			//스레드 생성
			h_thread = CreateThread(
				NULL,
				0,
				ThreadFunction,
				(LPVOID)h_Socket,
				0,
				NULL
			);

			//로그인 관련 창 비우기.
			system("cls");
			std::cout << "<Login Chatting Client>" << std::endl;
			std::cout << "로그인 성공. 채팅 시작." << std::endl;

			memset(buffer, 0, sizeof(buffer));
			//무한 recv
			while (recv(h_Socket, buffer, sizeof(buffer), 0) >= 0)
			{
				std::cout << " ->" << buffer << std::endl;
				memset(buffer, 0, sizeof(buffer));
			}
		}
	}

	/*
		서브 스레드 종료 확인.
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