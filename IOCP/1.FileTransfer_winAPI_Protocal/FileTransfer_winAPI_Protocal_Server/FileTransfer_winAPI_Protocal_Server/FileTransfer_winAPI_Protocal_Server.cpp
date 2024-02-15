#include <iostream>
#include <tchar.h>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

//TransmitFile() 사용을 위한 헤더
#include <Mswsock.h>
#pragma comment(lib, "Mswsock")

#include "Protocol.h"
#include <map>
#include <functional>
#include <vector>

/////////////////////////////////////////////////////////////////////////
//전송할 파일정보 개수
SEND_FILELIST g_flist = { 3 };

/////////////////////////////////////////////////////////////////////////
//클라이언트가 다운로드 가능한 파일
FILEINFO g_aFInfo[3] = {
	{ 0, L"Sleep Away.mp3", 4842585 },
	{ 1, L"Kalimba.mp3", 8414449 },
	{ 2, L"Maid with the Flaxen Hair.mp3", 4113874 }
};
std::vector<FILEINFO> g_vFInfo;

/////////////////////////////////////////////////////////////////////////
//언제든 에러가 나면 프로세스 종료.
void ErrorHandler(const char* pszMessage, SOCKET hSocket = INVALID_SOCKET, SOCKET hClient = INVALID_SOCKET, HANDLE hFile = INVALID_HANDLE_VALUE)
{
	printf("ERROR: %s\n", pszMessage);

	// 열려 있는 클라이언트 소켓이 있는 경우 닫기
	if (hClient != INVALID_SOCKET) {
		::closesocket(hClient);
	}

	// 서버 소켓이 열려 있는 경우 닫기
	if (hSocket != INVALID_SOCKET) {
		::closesocket(hSocket);
	}

	// 파일 핸들이 유효한 경우 닫기
	if (hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(hFile);
	}

	::WSACleanup();
	exit(1);
}

/////////////////////////////////////////////////////////////////////////
//파일 리스트 정보를 클라이언트에 전송하는 함수.
void SendFileList(SOCKET hClient)
{
	HEADER cmd;
	cmd.cmdCode = CMD_SEND_LIST;
	cmd.dwSize = sizeof(g_flist) + sizeof(FILEINFO) * g_vFInfo.size();
	//기본헤더 전송.
	send(hClient, (const char*)&cmd, sizeof(cmd), 0);
	//파일 리스트 헤더 전송.
	send(hClient, (const char*)&g_flist, sizeof(g_flist), 0);
	//파일 정보들 전송.
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
//인덱스에 해당하는 파일을 클라이언트에게 송신하는 함수.
void SendFile(SOCKET hClient, int index)
{
	HEADER cmd;
	ERROR_CODE err;
	//파일 리스트에서 인덱스에 맞는 파일을 검사한다.
	if (index < 0 || index > 2)
	{
		cmd.cmdCode = CMD_ERROR;
		cmd.dwSize = sizeof(err);
		err.nErrorCode = 0;
		strcpy_s(err.errorMessage, "잘못된 파일 인덱스 입니다.");

		//오류 정보를 클라이언트에게 전송.
		send(hClient, (const char*)&cmd, sizeof(cmd), 0);
		send(hClient, (const char*)&err, sizeof(err), 0);
		return;
	}

	// chat* -> wchar_t*
	WCHAR* name = g_vFInfo[index].szFileName;

	//전송할 파일 개방
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
		strcpy_s(err.errorMessage, "전송할 파일을 개방할 수 없습니다.");

		//오류 정보를 클라이언트에게 전송.
		send(hClient, (const char*)&cmd, sizeof(cmd), 0);
		send(hClient, (const char*)&err, sizeof(err), 0);
		ErrorHandler("전송할 파일을 개방할 수 없습니다.");
	}

	//전송할 파일에 대한 정보를 작성한다.
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

	//종료 신호 보내기
	HEADER pTail;
	pTail.cmdCode = CMD_FILE_END;
	pTail.dwSize = 0;
	tfb.Tail = &pTail;
	tfb.TailLength = sizeof(HEADER);

	//파일 송신
	if (::TransmitFile(
		hClient,	//파일을 전송할 소켓 핸들.
		hFile,		//전송할 파일 핸들.
		0,			//전송할 크기. 0이면 전체.
		65536,		//한 번에 전송할 버퍼 크기.
		NULL,		//비동기 입/출력에 대한 OVERLAPPED 구조체.
		&tfb,		//파일 전송에 앞,뒤로 전송할 데이터.
		0			//기타 옵션.
	) == FALSE)
		ErrorHandler("파일을 전송할 수 없습니다.");

	delete[] pCMD;

	::CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////////
// 함수 포인터 타입 정의, 파일 리스트의 인덱스를 런타임에 결정하기 때문에 효율성이 떨어진다.
//typedef std::function<void(SOCKET, unsigned int)> FuncPtr;
//
//// LOOKUP_TABLE 정의
//std::map<int, FuncPtr> LOOKUP_TABLE;

/////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	//룩업 테이블 정리.
	/*LOOKUP_TABLE[CMD_SEND_LIST] = [](SOCKET client, unsigned int index) {
									SendFileList(client);
									};
	LOOKUP_TABLE[CMD_GET_FILE] = SendFile;*/

	//파일 리스트 생성.
	std::wstring directoryPath = L"./FileList";
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		ErrorHandler("전송할 파일이 없습니다. 파일 리스트를 생성할 수 없습니다.");
	}

	unsigned int index = 0;
	do {
		// 파일이 디렉터리인지 확인
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

	//윈속 초기화
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		ErrorHandler("윈속을 초기화 할 수 없습니다.");

	//접속대기 소켓 생성
	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandler("접속 대기 소켓을 생성할 수 없습니다.");

	//포트 바인딩
	SOCKADDR_IN	svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket,
		(SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
		ErrorHandler("소켓에 IP주소와 포트를 바인드 할 수 없습니다.");

	//접속대기 상태로 전환
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
		ErrorHandler("리슨 상태로 전환할 수 없습니다.");
	puts("파일송신서버를 시작합니다.");

	//클라이언트 연결을 받아들이고 새로운 소켓 생성(개방)
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = ::accept(hSocket,
		(SOCKADDR*)&clientaddr, &nAddrLen);
	if (hClient == INVALID_SOCKET)
		ErrorHandler("클라이언트 통신 소켓을 생성할 수 없습니다.");
	puts("클라이언트가 연결되었습니다.");

	//클라이언트로부터 명령을 수신하고 대응하는 Event loop. -> 추후 룩업 테이블로 업데이트.
	HEADER cmd;
	while (::recv(hClient, (char*)&cmd, sizeof(HEADER), 0) > 0)
	{
		/*unsigned int index = -1;
		LOOKUP_TABLE[cmd.cmdCode](hClient, index);*/


		switch (cmd.cmdCode)
		{
		case CMD_GET_LIST:
			puts("클라이언트가 파일목록을 요구함.");
			SendFileList(hClient);
			break;

		case CMD_GET_FILE:
			puts("클라이언트가 파일전송을 요구함.");
			{
				GET_FILE file;
				::recv(hClient, (char*)&file, sizeof(file), 0);
				SendFile(hClient, file.nIndex);
				break;
			}

		default:
			ErrorHandler("알 수 없는 명령을 수신했습니다.");
			break;
		}
	}

	//클라이언트가 연결을 끊을 끊기를 대기한다.
	::recv(hClient, NULL, 0, 0);
	puts("클라이언트가 연결을 끊었습니다.");

	::closesocket(hClient);
	::closesocket(hSocket);
	::WSACleanup();
	return 0;
}

