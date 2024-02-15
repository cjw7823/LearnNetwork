#include <iostream>
#include <tchar.h>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>

#include "Protocol.h"

void ErrorHandler(const char* pszMessage)
{
	printf("ERROR: %s\n", pszMessage);
	::WSACleanup();
	exit(1);
}

/////////////////////////////////////////////////////////////////////////
void GetFileList(SOCKET hSocket)
{
	//서버에 파일 리스트를 요청한다.
	HEADER cmd = { CMD_GET_LIST, 0 };
	::send(hSocket, (const char*)&cmd, sizeof(cmd), 0);

	//서버로부터 파일 리스트를 수신한다.
	::recv(hSocket, (char*)&cmd, sizeof(cmd), 0);
	if (cmd.cmdCode != CMD_SEND_LIST)
		ErrorHandler("서버에서 파일 리스트를 수신하지 못했습니다.");

	SEND_FILELIST filelist;
	::recv(hSocket, (char*)&filelist, sizeof(filelist), 0);

	//수신한 리스트 정보를 화면에 출력한다.
	FILEINFO fInfo;
	for (unsigned int i = 0; i < filelist.nCount; ++i)
	{
		::recv(hSocket, (char*)&fInfo, sizeof(fInfo), 0);
		printf("%d\t%ls\t%d\n",
			fInfo.nIndex, fInfo.szFileName, fInfo.dwFileSize);
	}
}

/////////////////////////////////////////////////////////////////////////
void GetFile(SOCKET hSocket)
{
	int nIndex;
	printf("수신할 파일의 인덱스(0~2)를 입력하세요.: ");
	fflush(stdin);
	scanf_s("%d", &nIndex);

	//1. 서버에 파일 전송을 요청
	BYTE* pCommand = new BYTE[sizeof(HEADER) + sizeof(GET_FILE)];
	memset(pCommand, 0, sizeof(HEADER) + sizeof(GET_FILE));

	HEADER* pCmd = (HEADER*)pCommand;
	pCmd->cmdCode = CMD_GET_FILE;
	pCmd->dwSize = sizeof(GET_FILE);

	GET_FILE* pFile = (GET_FILE*)(pCommand + sizeof(HEADER));
	pFile->nIndex = nIndex;
	//두 헤더를 한 메모리에 묶어서 전송한다!
	::send(hSocket,
		(const char*)pCommand, sizeof(HEADER) + sizeof(GET_FILE), 0);
	delete[] pCommand;

	//2. 전송받을 파일에 대한 상세 정보 수신.
	HEADER cmd = { (CMDCODE)0, (DWORD)0 };
	FILEINFO fInfo = { 0 };
	::recv(hSocket, (char*)&cmd, sizeof(cmd), 0);
	if (cmd.cmdCode == CMD_ERROR)
	{
		ERROR_CODE err = { 0 };
		::recv(hSocket, (char*)&err, cmd.dwSize, 0);
		ErrorHandler(err.errorMessage);
	}
	else
		::recv(hSocket, (char*)&fInfo, cmd.dwSize, 0);

	//3. 파일을 수신한다.
	printf("%ls 파일 수신을 시작합니다!\n", fInfo.szFileName);
	HANDLE hFile = ::CreateFileW(
		fInfo.szFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//파일을 생성한다.
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		ErrorHandler("전송할 파일을 개방할 수 없습니다.");

	//서버가 전송하는 데이터를 반복해 파일에 붙여 넣는다.
	char byBuffer[65536];		//64KB
	int nRecv, recvLength;
	DWORD dwTotalRecv = 0, dwRead = 0;
	while (recvLength = fInfo.dwFileSize - dwTotalRecv)
	{
		if (recvLength >= 65536)
			recvLength = 65536;

		if ((nRecv = ::recv(hSocket, byBuffer, recvLength, 0)) > 0)
		{
			dwTotalRecv += nRecv;
			//서버에서 받은 크기만큼 데이터를 파일에 쓴다.
			::WriteFile(hFile, byBuffer, nRecv, &dwRead, NULL);
			printf("Receive: %d/%d\n", dwTotalRecv, fInfo.dwFileSize);
			fflush(stdout);
		}
		else
		{
			puts("ERROR: 파일 수신 중에 오류가 발생했습니다.");
			break;
		}
	}

	//파일 송신 종료신호 받기.
	HEADER end_signal;
	::recv(hSocket, (char*)&end_signal, sizeof(HEADER), 0);

	if(end_signal.cmdCode == CMD_FILE_END)
		printf("*** 파일수신이 끝났습니다. ***\n");
	else
		puts("ERROR: 파일 수신 중에 오류가 발생했습니다.");

	::CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		ErrorHandler("윈속을 초기화 할 수 없습니다.");

	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("소켓을 생성할 수 없습니다.");

	//포트 바인딩 및 연결
	SOCKADDR_IN	svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	inet_pton(AF_INET, "127.0.0.1", &svraddr.sin_addr.S_un.S_addr);
	if (::connect(hSocket,
		(SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
		ErrorHandler("서버에 연결할 수 없습니다.");

	//서버로부터 파일 리스트를 수신한다.
	GetFileList(hSocket);

	//전송받을 파일을 선택하고 수신한다.
	GetFile(hSocket);

	::closesocket(hSocket);
	::WSACleanup();
	return 0;
}
