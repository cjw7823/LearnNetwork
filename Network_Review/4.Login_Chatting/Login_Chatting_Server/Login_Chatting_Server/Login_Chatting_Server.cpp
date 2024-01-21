#include <iostream>

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
#endif

using namespace std;
using namespace sql;

int main()
{
	sql::Driver* driver = nullptr;
	sql::Connection* connection = nullptr;
	sql::Statement* statement = nullptr;
	sql::ResultSet* resultset = nullptr;
	sql::PreparedStatement* preparedstatement = nullptr;

	//workbench 열기
	driver = get_driver_instance();
	//연결 버튼 누르기
	try {
		// 연결 시도
		connection = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
	}
	catch (sql::SQLException& e) {
		cout << "SQLException caught: " << e.what() << " MySQL error code: " << e.getErrorCode();
		exit(-1);
	};

	//use Database
	connection->setSchema("Login_Chatting");
	//Query typing
	statement = connection->createStatement();

	while (1)
	{
		//로그인
		string userID;
		int Password;
		cout << "아이디 입력" << endl;
		cin >> userID;
		std::cin.clear(); // 플래그값 초기화 
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		cout << "비번 입력" << endl;
		cin >> Password;
		std::cin.clear(); // 플래그값 초기화 
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		//쿼리 실행

		//resultset = statement->executeQuery("select* from member");
		preparedstatement = connection->prepareStatement("select * from member where user_id = ?");//limit(어디서부터)(몇개 가져와라)
		preparedstatement->setString(1, userID);
		preparedstatement->execute();
		resultset = preparedstatement->getResultSet();

		for (unsigned int i = 0; i < resultset->rowsCount(); i++)
		{
			resultset->next();
			if (Password == resultset->getInt(2))//password 대조.
			{
				cout << "축하" << endl;
				exit(-1);
			}
		}
		cout << "로그인 실패" << endl;
	}
	

	//Display	
	//resultset = statement->executeQuery("select* from member");
	/*
	for (unsigned int i = 0; i < resultset->rowsCount(); i++)// 조건 중요. i<10으로 했는데 DB에 10개가 없다면 에러.
	{
		resultset->next();
		cout << resultset->getString("user_id") << endl;
	}
	*/

	/*while (resultset->next())
	{
		cout << resultset->getString(1)<<"  ";
		cout << resultset->getString(2) << endl;
	}*/

	delete resultset;
	delete statement;
	delete connection;

	return 0;
}