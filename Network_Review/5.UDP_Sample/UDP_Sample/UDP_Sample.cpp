#include "pch.h"

#include <Windows.h>
#pragma comment(lib, "ws2_32")

char g_RemoteAddress[32] = "127.0.0.1";
int g_nRemotePort = 25000;
int g_nLocalPort = 2500;

void ErrorHandler(const char* pszMessage, SOCKET hSocket = INVALID_SOCKET)
{
	printf("ERROR: %s\n", pszMessage);

	if (hSocket != INVALID_SOCKET) {
		::closesocket(hSocket);
	}

	::WSACleanup();
	exit(1);
}

DWORD WINAPI ThreadSendto(LPVOID pParam)
{
	//송신을 위한 UDP 소켓을 하나 더 개방한다.
	SOCKET hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("UDP 소켓을 생성할 수 없습니다.");

	char szBuffer[128];
	SOCKADDR_IN	remoteaddr = { 0 };
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_port = htons(g_nRemotePort);
	remoteaddr.sin_addr.S_un.S_addr = inet_addr(g_RemoteAddress);
	while (1)
	{
		gets_s(szBuffer);
		if (strcmp(szBuffer, "EXIT") == 0)
			break;

		//사용자가 입력한 메시지를 원격지로 전송한다.
		::sendto(hSocket, szBuffer, strlen(szBuffer) + 1, 0,
			(sockaddr*)&remoteaddr, sizeof(remoteaddr));
	}

	//수신을 위한 UDP 소켓을 닫는다. _tmain() 함수의 while문이 끝난다.
	::closesocket((SOCKET)pParam);
	::closesocket(hSocket);
	return 0;
}

int main()
{
    WSADATA wsaData = { 0, };
    if(::WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
		ErrorHandler("윈속을 초기화 할 수 없습니다.");

	SOCKET hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("UDP 소켓을 생성할 수 없습니다.");

	printf("원격지 IP주소를 입력하세요.: ");
	gets_s(g_RemoteAddress);
	fflush(stdin);
	printf("원격지 포트번호를 입력하세요.: ");
	scanf_s("%d", &g_nRemotePort);
	fflush(stdin);
	printf("로컬 포트번호를 입력하세요.: ");
	scanf_s("%d", &g_nLocalPort);

	char buffer[128];
	SOCKADDR_IN remoteAddr = { 0 };
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(g_nLocalPort);
	remoteAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket, (SOCKADDR*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR)
		ErrorHandler("소켓에 IP주소와 포트를 바인드 할 수 없습니다.");

	//메시지 송신 스레드 생성.
	DWORD dwThreadID = 0;
	HANDLE hThread = ::CreateThread(NULL,
		0,					//스택 메모리는 기본크기(1MB)
		ThreadSendto,		//스레드로 실행할 함수이름
		(LPVOID)hSocket,
		0,					//생성 플래그는 기본값 사용
		&dwThreadID);		//생성된 스레드ID가 저장될 변수주소
	::CloseHandle(hThread);

	//메시지 수신 및 출력.
	char szBuffer[128];
	SOCKADDR_IN	remoteaddr;
	int nLenSock = sizeof(remoteaddr), nResult;
	while ((nResult = ::recvfrom(hSocket, szBuffer, sizeof(szBuffer), 0,
		(sockaddr*)&remoteaddr, &nLenSock)) > 0)
	{
		printf("-> %s\n", szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
	}

	puts("UDP 통신 종료.");
	::closesocket(hSocket);
	::WSACleanup();

}