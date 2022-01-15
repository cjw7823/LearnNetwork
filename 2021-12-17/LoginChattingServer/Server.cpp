#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <vector>

#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/driver.h"
#include "jdbc/cppconn/connection.h"
#include "jdbc/cppconn/exception.h"
#include "jdbc/cppconn/resultset.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"

#ifdef _DEBUG
#pragma comment(lib, "./mysql_lib/lib/vs14/debug/mysqlcppconn.lib")
#else
#pragma comment(lib, "./mysql_lib/lib/vs14/mysqlcppconn.lib")
#endif

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI Broadcasting(void* arg);

CRITICAL_SECTION cs;

vector<SOCKET> vClientSocket;

sql::Driver* driver = nullptr;
sql::Connection* connection = nullptr;
sql::Statement* statement = nullptr;
sql::ResultSet* resultset = nullptr;
sql::PreparedStatement* preparedstatement = nullptr;

int main()
{
	cout << "Server" << endl;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerADDR;

	SOCKET ClientSocket;
	SOCKADDR_IN ClientADDR;

	InitializeCriticalSection(&cs);

	WSAData WsaData;

	//workbench 열기
	driver = get_driver_instance();
	//연결 버튼 누르기
	connection = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
	if (connection == nullptr)
	{
		cout << "connection error" << endl;
		exit(-1);
	}

	//use Database
	connection->setSchema("ygl");
	//Query typing
	statement = connection->createStatement();

	//Initailize Winsock 
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		cout << "error WSAStartup" << endl;
		exit(-1);
	}

	//서버 주소 구조체 초기화
	memset(&ServerADDR, 0, sizeof(ServerADDR));
	ServerADDR.sin_family = AF_INET;
	ServerADDR.sin_addr.s_addr = INADDR_ANY;
	ServerADDR.sin_port = htons(12345);

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

	bind(ServerSocket, (SOCKADDR*)&ServerADDR, sizeof(ServerADDR));

	listen(ServerSocket, 5);


	while (true)
	{
		int ClientAddrSize = sizeof(ClientADDR);
		ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientADDR, &ClientAddrSize);
		cout << "Connect : " << ClientSocket << endl;
		//ClientSocket을 기록
		EnterCriticalSection(&cs);
		vClientSocket.push_back(ClientSocket);
		LeaveCriticalSection(&cs);
		//worker thread
		HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, Broadcasting, (void*)&ClientSocket, 0, NULL);
		//ClientSocket 데이터 처리, recv, send(Thread)
	}

	closesocket(ServerSocket);

	DeleteCriticalSection(&cs);

	WSACleanup();

	return 0;
}

unsigned __stdcall Broadcasting(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;
	char Buffer[1024] = { 0, };

	//여기서 로그인 작업을?
	while (1)
	{
		char userID[20] = { 0, };
		char userPW[20] = { 0, };
		char checklogin[20] = { 0, };

		cout << "로그인 체크중.." << endl;
		recv(ClientSocket, userID, sizeof(userID), 0);
		cout << "ID Recived  "<< userID << endl;
		recv(ClientSocket, userPW, sizeof(userPW), 0);
		cout << "PW Recived  " << userPW << endl;

		preparedstatement = connection->prepareStatement("select* from member where user_id = ?");//limit(어디서부터)(몇개 가져와라)
		preparedstatement->setString(1, userID);
		preparedstatement->execute();
		resultset = preparedstatement->getResultSet();

		for (unsigned int i = 0; i < resultset->rowsCount(); i++)
		{
			resultset->next();
			if (userPW == resultset->getString(3))//아이디 찾기
			{
				cout << "축하" << endl;
				strcpy(checklogin, "Success");
				send(ClientSocket, checklogin, strlen(checklogin) + 1, 0);
				break;
			}
		}
		if (checklogin[0] == 'S')
		{
			break;
			
		}
		else
		{
			cout << "로그인 실패" << endl;
			strcpy(checklogin, "failed");
			send(ClientSocket, checklogin, strlen(checklogin) + 1, 0);
			continue;
		}
	}


	while (true)
	{
		int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)
		{
			//연결이 끊겼을때
			closesocket(ClientSocket);
			EnterCriticalSection(&cs);
			for (auto iter = vClientSocket.begin(); iter != vClientSocket.end(); ++iter)
			{
				if (*iter == ClientSocket)
				{
					vClientSocket.erase(iter);
					cout << "CloseSocket : " << ClientSocket << endl;
					break;
				}
			}
			LeaveCriticalSection(&cs);
		}
		else
		{
			EnterCriticalSection(&cs);
			for (auto iter = vClientSocket.begin(); iter != vClientSocket.end(); ++iter)
			{
				if (*iter != ClientSocket)
				{
					send(*iter, Buffer, strlen(Buffer) + 1, 0);
				}
			}
			LeaveCriticalSection(&cs);
		}
	}

	return 0;
}
