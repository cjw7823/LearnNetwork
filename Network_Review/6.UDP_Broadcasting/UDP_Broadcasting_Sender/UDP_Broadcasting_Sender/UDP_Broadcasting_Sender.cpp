#include "pch.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

void ErrorHandler(const char* pszMessage, SOCKET hSocket = INVALID_SOCKET)
{
	printf("ERROR: %s\n", pszMessage);

	if (hSocket != INVALID_SOCKET) {
		::closesocket(hSocket);
	}

	::WSACleanup();
	exit(1);
}

int main()
{
	//소켓 설정.
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		ErrorHandler("윈속을 초기화 할 수 없습니다.");

	SOCKET hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("UDP 소켓을 생성할 수 없습니다.");

	SOCKADDR_IN	addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25000);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);

	//브로드캐스트를 위해 소켓 옵션을 변경한다.
	int nOption = 1;
	::setsockopt(hSocket,
		SOL_SOCKET,
		SO_BROADCAST,			//브로드캐스트 설정.
		(const char*)&nOption,
		sizeof(nOption));

	char szBuffer[128] = { 0 };
	while (1)
	{
		putchar('->');
		gets_s(szBuffer);
		if (strcmp(szBuffer, "EXIT") == 0)		break;

		//모든 Peer들이 동시에 메시지를 수신.
		::sendto(hSocket, szBuffer, sizeof(szBuffer), 0,
			(const SOCKADDR*)&addr, sizeof(addr));

		//한 PC로 테스트할 경우.리시버의 포트가 같을 수 없으므로 포트번호별로 송신.
		/*
		int aPortNum[2] = { 25000, 25001 };
		for (int i = 0; i < 2; i++)
		{
			addr.sin_port = htons(aPortNum[i]);
			::sendto(hSocket, szBuffer, sizeof(szBuffer), 0,
				(const SOCKADDR*)&addr, sizeof(addr));
		}
		*/
	}

	::closesocket(hSocket);
	::WSACleanup();
}