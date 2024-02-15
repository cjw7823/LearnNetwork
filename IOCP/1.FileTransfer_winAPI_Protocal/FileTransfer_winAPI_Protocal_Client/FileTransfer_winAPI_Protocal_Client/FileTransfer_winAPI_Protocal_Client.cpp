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
	//������ ���� ����Ʈ�� ��û�Ѵ�.
	HEADER cmd = { CMD_GET_LIST, 0 };
	::send(hSocket, (const char*)&cmd, sizeof(cmd), 0);

	//�����κ��� ���� ����Ʈ�� �����Ѵ�.
	::recv(hSocket, (char*)&cmd, sizeof(cmd), 0);
	if (cmd.cmdCode != CMD_SEND_LIST)
		ErrorHandler("�������� ���� ����Ʈ�� �������� ���߽��ϴ�.");

	SEND_FILELIST filelist;
	::recv(hSocket, (char*)&filelist, sizeof(filelist), 0);

	//������ ����Ʈ ������ ȭ�鿡 ����Ѵ�.
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
	printf("������ ������ �ε���(0~2)�� �Է��ϼ���.: ");
	fflush(stdin);
	scanf_s("%d", &nIndex);

	//1. ������ ���� ������ ��û
	BYTE* pCommand = new BYTE[sizeof(HEADER) + sizeof(GET_FILE)];
	memset(pCommand, 0, sizeof(HEADER) + sizeof(GET_FILE));

	HEADER* pCmd = (HEADER*)pCommand;
	pCmd->cmdCode = CMD_GET_FILE;
	pCmd->dwSize = sizeof(GET_FILE);

	GET_FILE* pFile = (GET_FILE*)(pCommand + sizeof(HEADER));
	pFile->nIndex = nIndex;
	//�� ����� �� �޸𸮿� ��� �����Ѵ�!
	::send(hSocket,
		(const char*)pCommand, sizeof(HEADER) + sizeof(GET_FILE), 0);
	delete[] pCommand;

	//2. ���۹��� ���Ͽ� ���� �� ���� ����.
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

	//3. ������ �����Ѵ�.
	printf("%ls ���� ������ �����մϴ�!\n", fInfo.szFileName);
	HANDLE hFile = ::CreateFileW(
		fInfo.szFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//������ �����Ѵ�.
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		ErrorHandler("������ ������ ������ �� �����ϴ�.");

	//������ �����ϴ� �����͸� �ݺ��� ���Ͽ� �ٿ� �ִ´�.
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
			//�������� ���� ũ�⸸ŭ �����͸� ���Ͽ� ����.
			::WriteFile(hFile, byBuffer, nRecv, &dwRead, NULL);
			printf("Receive: %d/%d\n", dwTotalRecv, fInfo.dwFileSize);
			fflush(stdout);
		}
		else
		{
			puts("ERROR: ���� ���� �߿� ������ �߻��߽��ϴ�.");
			break;
		}
	}

	//���� �۽� �����ȣ �ޱ�.
	HEADER end_signal;
	::recv(hSocket, (char*)&end_signal, sizeof(HEADER), 0);

	if(end_signal.cmdCode == CMD_FILE_END)
		printf("*** ���ϼ����� �������ϴ�. ***\n");
	else
		puts("ERROR: ���� ���� �߿� ������ �߻��߽��ϴ�.");

	::CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		ErrorHandler("������ �ʱ�ȭ �� �� �����ϴ�.");

	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("������ ������ �� �����ϴ�.");

	//��Ʈ ���ε� �� ����
	SOCKADDR_IN	svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	inet_pton(AF_INET, "127.0.0.1", &svraddr.sin_addr.S_un.S_addr);
	if (::connect(hSocket,
		(SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
		ErrorHandler("������ ������ �� �����ϴ�.");

	//�����κ��� ���� ����Ʈ�� �����Ѵ�.
	GetFileList(hSocket);

	//���۹��� ������ �����ϰ� �����Ѵ�.
	GetFile(hSocket);

	::closesocket(hSocket);
	::WSACleanup();
	return 0;
}
