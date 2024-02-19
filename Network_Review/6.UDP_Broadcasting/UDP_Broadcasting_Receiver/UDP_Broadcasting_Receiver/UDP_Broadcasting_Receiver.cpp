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
		ErrorHandler("UDP 소켓을 생성할 수 없습니다.", hSocket);

	SOCKADDR_IN	addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25000);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		puts("ERROR: 소켓에 IP주소와 포트를 바인드 할 수 없습니다.");
		return 0;
	}

	//메시지를 수신하고 화면에 출력한다.
	char szBuffer[128] = { 0 };
	while (::recvfrom(hSocket,
		szBuffer, sizeof(szBuffer), 0, NULL, 0) >= 0)
	{
		printf(":%s\n", szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
	}

	::closesocket(hSocket);
	::WSACleanup();
}
