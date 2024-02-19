#include "pch.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

#include <list>
#include <iterator>


typedef struct _USERSESSION
{
	SOCKET	hSocket;
	char	buffer[8192];	//8KB
} USER_SESSION;

//클라이언트 처리를 위한 Worker스레드 개수.
//서버 특성에 따라 개수 최적화 필요.
#define MAX_THREAD_CNT	4

CRITICAL_SECTION  g_cs;				//스레드 동기화 객체
std::list<SOCKET>	g_listClient;	//연결된 클라이언트 소켓 리스트.
SOCKET	g_hSocket;					//서버의 리슨 소켓
HANDLE	g_hIocp;					//IOCP 핸들

void ReleaseServer();
void CloseClient(SOCKET hSock);
void ErrorHandler(const char* pszMessage);
BOOL CtrlHandler(DWORD dwType);
void SendMessageAll(char* pszMessage, int nSize);

DWORD WINAPI IOCP_Accept_Thread(LPVOID pParam);
DWORD WINAPI IOCP_Worker_Thread(LPVOID pParam);

int main()
{
	WSADATA wsaData = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandler("윈속을 초기화 할 수 없습니다.");

	::InitializeCriticalSection(&g_cs);

	//Ctrl+C 이벤트 함수 등록.
	if (::SetConsoleCtrlHandler(
		(PHANDLER_ROUTINE)CtrlHandler, TRUE) == FALSE)
		puts("ERROR: Ctrl+C 처리기를 등록할 수 없습니다.");

	/*
		IOCP 생성.
		CreateIoCompletionPort 함수는 다음 두가지 경우에 모두 쓰인다.
		1. IOCP Queue를 만들며 IOCP 로직 구축.
		2. IOCP Queue에 등록하여 관리.
	*/
	g_hIocp = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,	//연결된 파일 없음.
		NULL,			//기존 핸들 없음.
		0,				//식별자(Key) 해당되지 않음.
		0);				//스레드 개수는 OS에 맡김.
	if (g_hIocp == NULL)
		ErrorHandler("IOCP를 생성할 수 없습니다.");

	//IOCP Worker 스레드들 생성
	HANDLE hThread;
	DWORD dwThreadID;
	for (int i = 0; i < MAX_THREAD_CNT; ++i)
	{
		dwThreadID = 0;
		hThread = ::CreateThread(NULL,	//보안속성 상속
			0,							//스택 메모리는 기본크기(1MB)
			IOCP_Worker_Thread,			//스래드 함수
			(LPVOID)NULL,				//매개변수
			0,							//생성 플래그는 기본값 사용
			&dwThreadID);				//생성된 스레드ID가 저장될 변수주소

		::CloseHandle(hThread);
	}

	//서버 리슨 소켓 생성.
	//IOCP용 소켓을 생성해야 하므로 WSASocket 사용.
	g_hSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN addrsvr;
	addrsvr.sin_family = AF_INET;
	addrsvr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
	addrsvr.sin_port = ::htons(25000);

	if (::bind(g_hSocket,
		(SOCKADDR*)&addrsvr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		ErrorHandler("포트가 이미 사용중입니다.");

	if (::listen(g_hSocket, SOMAXCONN) == SOCKET_ERROR)
		ErrorHandler("리슨 상태로 전환할 수 없습니다.");

	//반복해서 클라이언트의 연결을 accept().
	hThread = ::CreateThread(NULL, 0, IOCP_Accept_Thread,
		(LPVOID)NULL, 0, &dwThreadID);
	::CloseHandle(hThread);

	//메인 함수가 반환하지 않도록 대기.
	puts("[채팅서버 시작]");
	while (1)
		getchar();
}


/// <summary>
/// 클라이언트 소켓, 리슨 소켓, IOCP 핸들을 닫는다.
/// </summary>
void ReleaseServer()
{
	//클라이언트 소켓 닫기.
	std::list<SOCKET>::iterator it;

	::EnterCriticalSection(&g_cs);
	for (it = g_listClient.begin(); it != g_listClient.end(); ++it)
	{
		::shutdown(*it, SD_BOTH);
		::closesocket(*it);
	}
	::LeaveCriticalSection(&g_cs);

	//Listen 소켓을 닫기.
	::shutdown(g_hSocket, SD_BOTH);
	::closesocket(g_hSocket);
	g_hSocket = NULL;

	//IOCP 핸들을 닫는다. 이렇게 하면 GetQueuedCompletionStatus() 함수가 FALSE를 반환,
	//:GetLastError() 함수가 ERROR_ABANDONED_WAIT_0을 반환한다. IOCP 스레드들이 모두 종료된다.
	::CloseHandle(g_hIocp);
	g_hIocp = NULL;

	//IOCP 스레드 종료 대기.
	::Sleep(500);
	::DeleteCriticalSection(&g_cs);
}

/// <summary>
/// 개별 클라이언트 종료.
/// </summary>
/// <param name="hSock"> 종료할 클라이언트 소켓. </param>
void CloseClient(SOCKET hSock)
{
	::shutdown(hSock, SD_BOTH);
	::closesocket(hSock);

	::EnterCriticalSection(&g_cs);
	g_listClient.remove(hSock);
	::LeaveCriticalSection(&g_cs);
}

/// <summary>
/// API 호출 중 에러 발생시 종료 로직.
/// </summary>
/// <param name="화면에 출력할 에러 메세지."></param>
void ErrorHandler(const char* pszMessage)
{
	ReleaseServer();

	printf("ERROR: %s\n", pszMessage);
	::WSACleanup();
	exit(1);
}

/// <summary>
/// Ctrl+C 이벤트로 인한 프로그램 종료로직.
/// </summary>
/// <param name="dwType"> 입력된 이벤트 타입. </param>
/// <returns> 정상이면 TRUE. </returns>
BOOL CtrlHandler(DWORD dwType)
{
	if (dwType == CTRL_C_EVENT)
	{
		ReleaseServer();

		puts("[채팅서버 종료]");
		::WSACleanup();
		exit(0);
		return TRUE;
	}

	return FALSE;
}

/// <summary>
/// 모든 클라이언트에 메세지 송신.
/// </summary>
/// <param name="pszMessage">전송할 메세지.</param>
/// <param name="nSize">메세지 길이.</param>
void SendMessageAll(char* pszMessage, int nSize)
{
	std::list<SOCKET>::iterator it;

	::EnterCriticalSection(&g_cs);
	for (it = g_listClient.begin(); it != g_listClient.end(); ++it)
		::send(*it, pszMessage, nSize, 0);
	::LeaveCriticalSection(&g_cs);
}

/// <summary>
/// 리슨소켓의 Accpet 이후 로직 처리.
/// 1. IOCP Queue에 소켓 등록.
/// 2. 클라이언트 소켓 비동기 수신.
/// </summary>
/// <param name="pParam">사용하지 않음.</param>
/// <returns></returns>
DWORD __stdcall IOCP_Accept_Thread(LPVOID pParam)
{
	LPWSAOVERLAPPED	pWol = NULL;
	DWORD			dwReceiveSize, dwFlag;
	USER_SESSION*	pNewUser;
	int				nAddrSize = sizeof(SOCKADDR);
	WSABUF			wsaBuf;
	SOCKADDR		ClientAddr;
	SOCKET			hClient;
	int				nRecvResult = 0;

	while ((hClient = ::accept(g_hSocket,
		&ClientAddr, &nAddrSize)) != INVALID_SOCKET)
	{
		puts("새 클라이언트가 연결됐습니다.");
		::EnterCriticalSection(&g_cs);
		g_listClient.push_back(hClient);
		::LeaveCriticalSection(&g_cs);

		//새 클라이언트에 대한 세션 객체 생성
		pNewUser = new USER_SESSION;
		::ZeroMemory(pNewUser, sizeof(USER_SESSION));
		pNewUser->hSocket = hClient;

		//비동기 수신 처리를 위한 OVERLAPPED 구조체 생성.
		pWol = new WSAOVERLAPPED;
		::ZeroMemory(pWol, sizeof(WSAOVERLAPPED));

		//클라이언트 소켓 핸들을 IOCP에 연결.
		::CreateIoCompletionPort((HANDLE)hClient, g_hIocp,
			(ULONG_PTR)pNewUser,		//IOCP Queue에 들어갈 소켓들을 구분할 키.
			0);

		dwReceiveSize = 0;
		dwFlag = 0;
		wsaBuf.buf = pNewUser->buffer;
		wsaBuf.len = sizeof(pNewUser->buffer);

		//클라이언트가 보낸 정보를 비동기 수신한다.
		nRecvResult = ::WSARecv(hClient, &wsaBuf, 1, &dwReceiveSize,
			&dwFlag, pWol, NULL);
		if (::WSAGetLastError() != WSA_IO_PENDING)
			puts("ERROR: WSARecv() != WSA_IO_PENDING");
	}

	return 0;
}

/// <summary>
/// IOCP Queue 에서 Dequeue하여 데이터를 처리.
/// </summary>
/// <param name="pParam">사용하지 않음.</param>
/// <returns></returns>
DWORD __stdcall IOCP_Worker_Thread(LPVOID pParam)
{
	DWORD			dwTransferredSize = 0;
	DWORD			dwFlag = 0;
	USER_SESSION*	pSession = NULL;
	LPWSAOVERLAPPED	pWol = NULL;
	BOOL			bResult;	

	puts("[IOCP 작업자 스레드 시작]");
	while (1)
	{
		bResult = ::GetQueuedCompletionStatus(
			g_hIocp,				//Dequeue할 IOCP 핸들.
			&dwTransferredSize,		//수신한 데이터 크기.
			(PULONG_PTR)&pSession,	//수신된 데이터의 키, USER_SESSION 설계 상 저장된 메모리를 의미.
			&pWol,					//OVERLAPPED 구조체.
			INFINITE);				//이벤트를 무한정 대기.

		if (bResult == TRUE)
		{
			//정상적인 경우.

			//1. 클라이언트가 소켓을 정상적으로 닫고 연결을 끊은 경우.
			if (dwTransferredSize == 0)
			{

				CloseClient(pSession->hSocket);
				delete pWol;
				delete pSession;
				puts("\tGQCS: 클라이언트가 정상적으로 연결을 종료함.");
			}

			//2. 클라이언트가 보낸 데이터를 수신한 경우.
			else
			{
				SendMessageAll(pSession->buffer, dwTransferredSize);
				memset(pSession->buffer, 0, sizeof(pSession->buffer));

				//다시 IOCP Queue에 등록.
				DWORD dwReceiveSize = 0;
				DWORD dwFlag = 0;
				WSABUF wsaBuf = { 0 };
				wsaBuf.buf = pSession->buffer;
				wsaBuf.len = sizeof(pSession->buffer);

				::WSARecv(
					pSession->hSocket,	//클라이언트 소켓 핸들
					&wsaBuf,			//WSABUF 구조체 배열의 주소
					1,					//배열 요소의 개수
					&dwReceiveSize,
					&dwFlag,
					pWol,
					NULL);
				if (::WSAGetLastError() != WSA_IO_PENDING)
					puts("\tGQCS: ERROR: WSARecv()");
			}
		}
		else
		{
			//비정상적인 경우.

			//3. 큐에서 완료 패킷을 꺼내지 못하고 반환한 경우.
			if (pWol == NULL)
			{
				//IOCP 핸들이 닫힌 경우(서버를 종료하는 경우)도 해당된다.
				puts("\tGQCS: IOCP 핸들이 닫혔습니다.");
				break;
			}

			//4. 클라이언트가 비정상적으로 종료됐거나
			//   서버가 먼저 연결을 종료한 경우.
			else
			{
				if (pSession != NULL)
				{
					CloseClient(pSession->hSocket);
					delete pWol;
					delete pSession;
				}

				puts("\tGQCS: 서버 종료 혹은 비정상적 연결 종료");
			}
		}
	}

	puts("[IOCP 작업자 스레드 종료]");
	return 0;
}