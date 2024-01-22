/*
	Release 모드로 실행하세요.

	MySQL c++ Connector 라이브러리의 Debug 파일이 100MB를 넘습니다.
	Git LFS 사용제한을 위해 Release모드로 작성합니다.
*/


#include <iostream>
#include <WinSock2.h>
#include <list>
#include <WS2tcpip.h>

#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/driver.h"
#include "jdbc/cppconn/connection.h"
#include "jdbc/cppconn/exception.h"
#include "jdbc/cppconn/resultset.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"

#ifdef _DEBUG
#pragma comment(lib, "vs14/debug/mysqlcppconn.lib")
#pragma comment(lib, "vs14/debug/mysqlcppconn-static.lib")
#else
#pragma comment(lib, "vs14/mysqlcppconn.lib")
#pragma comment(lib, "vs14/mysqlcppconn-static.lib")
#endif
#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace sql;

sql::Driver* driver = nullptr;
sql::Connection* connection = nullptr;
sql::Statement* statement = nullptr;
sql::ResultSet* resultset = nullptr;
sql::PreparedStatement* preparedstatement = nullptr;

SOCKET g_serverSocket;
CRITICAL_SECTION g_cs;
list<SOCKET> g_socketList;

int BroadCastMessage(SOCKET soc)
{
	char buffer[1024] = { 0, };
	while (recv(soc, buffer, sizeof(buffer), 0) >= 0)
	{
		//모든 클라이언트에 메세지 전송
		int len = strlen(buffer);
		EnterCriticalSection(&g_cs);
		for (auto it = g_socketList.begin(); it != g_socketList.end(); it++)
		{
			if(*it != soc)//메세지를 전송한 클라이언트는 제외.
			send(*it, buffer, sizeof(char) * (len + 1), 0);
		}
		LeaveCriticalSection(&g_cs);

		memset(buffer, 0, sizeof(buffer));
	}

	return -1;
}

DWORD WINAPI ThreadFunction(LPVOID param)
{
	SOCKET h_socket = (SOCKET)param;
	printf("클라이언트 접속 : %llu \n", h_socket);

	/*
		클라이언트로부터 ID와 PW를 입력받아 로그인 시도.
	*/
	char buffer[1024] = { 0, };
	while (recv(h_socket, buffer, sizeof(buffer), 0) >= 0) //ID / PW 수신.
	{
		// ID/PW 파싱.
		string str = buffer;
		string userID = str.substr(0, str.find('/'));
		string Password = str.substr(str.find('/') + 1);
		memset(buffer, 0, sizeof(buffer));

		//쿼리 실행
		preparedstatement = connection->prepareStatement("select * from member where user_id = ?");//limit(어디서부터)(몇개 가져와라)
		preparedstatement->setString(1, userID);
		preparedstatement->execute();
		resultset = preparedstatement->getResultSet();

		if (resultset->rowsCount() > 0) //일치하는 iD가 존재.
		{
			for (unsigned int i = 0; i < resultset->rowsCount(); i++)// ID가 key이기 때문에 사실상 rowCount == 1.
			{
				resultset->next();
				if (stoi(Password) == resultset->getInt(2))	//로그인 성공
				{
					::send(h_socket, "Login", sizeof("Login"), 0);
					if (BroadCastMessage(h_socket) == -1)
						goto end;
				}
			}
		}
		::send(h_socket, "Login Failed", sizeof("Login Failed"), 0); //로그인 실패. ID나 PW가 없음.
	}

end:
	EnterCriticalSection(&g_cs);
	g_socketList.remove(h_socket);
	LeaveCriticalSection(&g_cs);
	printf("클라이언트 접속 해제 : %llu \n", h_socket);

	return 0;
}

int main()
{
	cout << "<Login Chatting Server>" << endl;

	/*
		DB Server 접속
	*/
	driver = get_driver_instance();
	try
	{
		// 연결 시도
		connection = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
	}
	catch (sql::SQLException& e)
	{
		cout << "SQLException caught: " << e.what() << " MySQL error code: " << e.getErrorCode();
		exit(-1);
	};

	//use Database
	connection->setSchema("Login_Chatting");
	//Query typing
	statement = connection->createStatement();

	/*
		소켓 설정. 가독성을 위해 예외처리 생략.
	*/
	WSADATA wsaData = { 0, };
	::WSAStartup(MAKEWORD(2, 2), &wsaData);

	g_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(8080);
	::inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr);
	::bind(g_serverSocket, (sockaddr*)&addrServer, sizeof(addrServer));
	::listen(g_serverSocket, SOMAXCONN);

	InitializeCriticalSection(&g_cs);

	SOCKET h_clientSocket;
	while ((h_clientSocket = ::accept(g_serverSocket, NULL, NULL)) != INVALID_SOCKET)
	{
		HANDLE h_thread = CreateThread(
			NULL,
			0,
			ThreadFunction,
			(LPVOID)h_clientSocket,
			0,
			NULL
		);

		EnterCriticalSection(&g_cs);
		g_socketList.push_back(h_clientSocket);
		LeaveCriticalSection(&g_cs);
	}

	delete resultset;
	delete statement;
	delete connection;

	for (auto it = g_socketList.begin(); it != g_socketList.end(); it++)
		closesocket(*it);
	DeleteCriticalSection(&g_cs);
	WSACleanup();

	return 0;
}