#include <iostream>

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

using namespace std;
using namespace sql;

int main()
{
	sql::Driver* driver = nullptr;
	sql::Connection* connection = nullptr;
	sql::Statement* statement = nullptr;
	sql::ResultSet* resultset = nullptr;
	sql::PreparedStatement* preparedstatement = nullptr;


	//workbench ����
	driver = get_driver_instance();
	//���� ��ư ������
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

	while (1)
	{
		//�α���
		string userID;
		int Password;
		cout << "���̵� �Է�" << endl;
		cin >> userID;
		cout << "��� �Է�" << endl;
		cin >> Password;

		//���� ����

		//resultset = statement->executeQuery("select* from member");
		preparedstatement = connection->prepareStatement("select* from member where user_id = ?");//limit(��𼭺���)(� �����Ͷ�)
		preparedstatement->setString(1, userID);
		preparedstatement->execute();
		resultset = preparedstatement->getResultSet();

		for (unsigned int i = 0; i < resultset->rowsCount(); i++)
		{
			resultset->next();
			if (Password == resultset->getInt(3))//���̵� ã��
			{
				cout << "����" << endl;
				exit(-1);
			}
		}
		cout << "�α��� ����" << endl;

	}
	//Display
	//for (unsigned int i = 0; i < resultset->rowsCount(); i++)// ���� �߿�. i<10���� �ߴµ� DB�� 10���� ���ٸ� ����.
	//{
	//	resultset->next();
	//	cout << resultset->getString("user_id") << endl;
	//}

	//while (resultset->next())
	//{
	//	cout << resultset->getString(1)<<"  ";
	//	cout << resultset->getString(2) << "  ";
	//	cout << resultset->getString(3) << endl;
	//}


	delete resultset;
	delete statement;
	delete connection;


	return 0;
}