#include <iostream>
#include <tchar.h>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

//TransmitFile() ����� ���� ���
#include <Mswsock.h>
#pragma comment(lib, "Mswsock")

#include "Protocol.h"
#include <map>
#include <functional>
#include <vector>

/////////////////////////////////////////////////////////////////////////
//������ �������� ����
SEND_FILELIST g_flist = { 3 };

/////////////////////////////////////////////////////////////////////////
//Ŭ���̾�Ʈ�� �ٿ�ε� ������ ����
FILEINFO g_aFInfo[3] = {
	{ 0, L"Sleep Away.mp3", 4842585 },
	{ 1, L"Kalimba.mp3", 8414449 },
	{ 2, L"Maid with the Flaxen Hair.mp3", 4113874 }
};
std::vector<FILEINFO> g_vFInfo;

/////////////////////////////////////////////////////////////////////////
//������ ������ ���� ���μ��� ����.
void ErrorHandler(const char* pszMessage, SOCKET hSocket = INVALID_SOCKET, SOCKET hClient = INVALID_SOCKET, HANDLE hFile = INVALID_HANDLE_VALUE)
{
	printf("ERROR: %s\n", pszMessage);

	// ���� �ִ� Ŭ���̾�Ʈ ������ �ִ� ��� �ݱ�
	if (hClient != INVALID_SOCKET) {
		::closesocket(hClient);
	}

	// ���� ������ ���� �ִ� ��� �ݱ�
	if (hSocket != INVALID_SOCKET) {
		::closesocket(hSocket);
	}

	// ���� �ڵ��� ��ȿ�� ��� �ݱ�
	if (hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(hFile);
	}

	::WSACleanup();
	exit(1);
}

/////////////////////////////////////////////////////////////////////////
//���� ����Ʈ ������ Ŭ���̾�Ʈ�� �����ϴ� �Լ�.
void SendFileList(SOCKET hClient)
{
	HEADER cmd;
	cmd.cmdCode = CMD_SEND_LIST;
	cmd.dwSize = sizeof(g_flist) + sizeof(FILEINFO) * g_vFInfo.size();
	//�⺻��� ����.
	send(hClient, (const char*)&cmd, sizeof(cmd), 0);
	//���� ����Ʈ ��� ����.
	send(hClient, (const char*)&g_flist, sizeof(g_flist), 0);
	//���� ������ ����.
	for (int i = 0; i < g_vFInfo.size(); i++)
	{
		FILEINFO file;
		file.nIndex = g_vFInfo[i].nIndex;
		wcscpy_s(file.szFileName, g_vFInfo[i].szFileName);
		file.dwFileSize = g_vFInfo[i].dwFileSize;
		send(hClient, (const char*)&file, sizeof(file), 0);
	}
	//send(hClient, (const char*)g_aFInfo, sizeof(g_aFInfo), 0);
}

/////////////////////////////////////////////////////////////////////////
//�ε����� �ش��ϴ� ������ Ŭ���̾�Ʈ���� �۽��ϴ� �Լ�.
void SendFile(SOCKET hClient, int index)
{
	HEADER cmd;
	ERROR_CODE err;
	//���� ����Ʈ���� �ε����� �´� ������ �˻��Ѵ�.
	if (index < 0 || index > 2)
	{
		cmd.cmdCode = CMD_ERROR;
		cmd.dwSize = sizeof(err);
		err.nErrorCode = 0;
		strcpy_s(err.errorMessage, "�߸��� ���� �ε��� �Դϴ�.");

		//���� ������ Ŭ���̾�Ʈ���� ����.
		send(hClient, (const char*)&cmd, sizeof(cmd), 0);
		send(hClient, (const char*)&err, sizeof(err), 0);
		return;
	}

	// chat* -> wchar_t*
	WCHAR* name = g_vFInfo[index].szFileName;

	//������ ���� ����
	HANDLE hFile = ::CreateFile(name,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cmd.cmdCode = CMD_ERROR;
		cmd.dwSize = sizeof(err);
		err.nErrorCode = -1;
		memset(err.errorMessage, 0, _MAX_FNAME);
		strcpy_s(err.errorMessage, "������ ������ ������ �� �����ϴ�.");

		//���� ������ Ŭ���̾�Ʈ���� ����.
		send(hClient, (const char*)&cmd, sizeof(cmd), 0);
		send(hClient, (const char*)&err, sizeof(err), 0);
		ErrorHandler("������ ������ ������ �� �����ϴ�.");
	}

	//������ ���Ͽ� ���� ������ �ۼ��Ѵ�.
	DWORD header_length = sizeof(HEADER) + sizeof(FILEINFO);
	BYTE* pCMD = new BYTE[header_length];
	memset(pCMD, 0, header_length);

	HEADER* pHeader = (HEADER*)pCMD;
	pHeader->cmdCode = CMD_FILE_BEGIN;
	pHeader->dwSize = sizeof(FILEINFO);

	FILEINFO* pFileInfo = (FILEINFO*)(pCMD + sizeof(HEADER));
	pFileInfo->nIndex = g_vFInfo[index].nIndex;
	wcscpy_s(pFileInfo->szFileName, g_vFInfo[index].szFileName);
	pFileInfo->dwFileSize = ::GetFileSize(hFile, NULL);

	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = pCMD;
	tfb.HeadLength = header_length;

	//���� ��ȣ ������
	HEADER pTail;
	pTail.cmdCode = CMD_FILE_END;
	pTail.dwSize = 0;
	tfb.Tail = &pTail;
	tfb.TailLength = sizeof(HEADER);

	//���� �۽�
	if (::TransmitFile(
		hClient,	//������ ������ ���� �ڵ�.
		hFile,		//������ ���� �ڵ�.
		0,			//������ ũ��. 0�̸� ��ü.
		65536,		//�� ���� ������ ���� ũ��.
		NULL,		//�񵿱� ��/��¿� ���� OVERLAPPED ����ü.
		&tfb,		//���� ���ۿ� ��,�ڷ� ������ ������.
		0			//��Ÿ �ɼ�.
	) == FALSE)
		ErrorHandler("������ ������ �� �����ϴ�.");

	delete[] pCMD;

	::CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////////
// �Լ� ������ Ÿ�� ����, ���� ����Ʈ�� �ε����� ��Ÿ�ӿ� �����ϱ� ������ ȿ������ ��������.
//typedef std::function<void(SOCKET, unsigned int)> FuncPtr;
//
//// LOOKUP_TABLE ����
//std::map<int, FuncPtr> LOOKUP_TABLE;

/////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	//��� ���̺� ����.
	/*LOOKUP_TABLE[CMD_SEND_LIST] = [](SOCKET client, unsigned int index) {
									SendFileList(client);
									};
	LOOKUP_TABLE[CMD_GET_FILE] = SendFile;*/

	//���� ����Ʈ ����.
	std::wstring directoryPath = L"./FileList";
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		ErrorHandler("������ ������ �����ϴ�. ���� ����Ʈ�� ������ �� �����ϴ�.");
	}

	unsigned int index = 0;
	do {
		// ������ ���͸����� Ȯ��
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			std::wcout << findFileData.cFileName << std::endl;
			FILEINFO file;
			file.nIndex = index++;

			WCHAR fileName[MAX_PATH];
			wcscpy_s(fileName, L"./FileList/");
			wcscat_s(fileName, findFileData.cFileName);
			wcscpy_s(file.szFileName, fileName);

			file.dwFileSize = (static_cast<unsigned long long>(findFileData.nFileSizeHigh) << 32) | findFileData.nFileSizeLow;
			g_vFInfo.push_back(file);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
	
	g_flist.nCount = g_vFInfo.size();

	//���� �ʱ�ȭ
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		ErrorHandler("������ �ʱ�ȭ �� �� �����ϴ�.");

	//���Ӵ�� ���� ����
	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("���� ��� ������ ������ �� �����ϴ�.");

	//��Ʈ ���ε�
	SOCKADDR_IN	svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket,
		(SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
		ErrorHandler("���Ͽ� IP�ּҿ� ��Ʈ�� ���ε� �� �� �����ϴ�.");

	//���Ӵ�� ���·� ��ȯ
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
		ErrorHandler("���� ���·� ��ȯ�� �� �����ϴ�.");
	puts("���ϼ۽ż����� �����մϴ�.");

	//Ŭ���̾�Ʈ ������ �޾Ƶ��̰� ���ο� ���� ����(����)
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = ::accept(hSocket,
		(SOCKADDR*)&clientaddr, &nAddrLen);
	if (hClient == INVALID_SOCKET)
		ErrorHandler("Ŭ���̾�Ʈ ��� ������ ������ �� �����ϴ�.");
	puts("Ŭ���̾�Ʈ�� ����Ǿ����ϴ�.");

	//Ŭ���̾�Ʈ�κ��� ����� �����ϰ� �����ϴ� Event loop. -> ���� ��� ���̺�� ������Ʈ.
	HEADER cmd;
	while (::recv(hClient, (char*)&cmd, sizeof(HEADER), 0) > 0)
	{
		/*unsigned int index = -1;
		LOOKUP_TABLE[cmd.cmdCode](hClient, index);*/


		switch (cmd.cmdCode)
		{
		case CMD_GET_LIST:
			puts("Ŭ���̾�Ʈ�� ���ϸ���� �䱸��.");
			SendFileList(hClient);
			break;

		case CMD_GET_FILE:
			puts("Ŭ���̾�Ʈ�� ���������� �䱸��.");
			{
				GET_FILE file;
				::recv(hClient, (char*)&file, sizeof(file), 0);
				SendFile(hClient, file.nIndex);
				break;
			}

		default:
			ErrorHandler("�� �� ���� ����� �����߽��ϴ�.");
			break;
		}
	}

	//Ŭ���̾�Ʈ�� ������ ���� ���⸦ ����Ѵ�.
	::recv(hClient, NULL, 0, 0);
	puts("Ŭ���̾�Ʈ�� ������ �������ϴ�.");

	::closesocket(hClient);
	::closesocket(hSocket);
	::WSACleanup();
	return 0;
}

