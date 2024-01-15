#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <stdio.h>
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{

	SOCKADDR_IN Addr;

	WSAData wsaData;

	struct hostent* host;
	struct hostent* host2;

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup");
		exit(-1);
	}

	while (1)
	{
		char domain[] = "";
		printf("도메인이나 아이피를 입력하세요.\n");
		cin >> domain;

		host = gethostbyname((char*)domain);

		memset(&Addr, 0, sizeof(SOCKADDR_IN));
		Addr.sin_addr.s_addr = inet_addr((char*)domain);

		host2 = gethostbyaddr((char*)&Addr.sin_addr, 4, AF_INET);

		if (host != nullptr)
		{
			printf("official : %s\n", host->h_name);

			for (int i = 0; host->h_aliases[i]; i++)
			{
				printf("alias : %s\n", host->h_aliases[i]);
			}

			printf("AF : %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

			for (int i = 0; host->h_addr_list[i]; i++)
			{
				printf("ip addr : %s\n", inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]));
			}
		}

		else if (host2 != nullptr)
		{
			for (int i = 0; host2->h_aliases[i]; i++)
			{
				printf("alias : %s\n", host2->h_aliases[i]);
			}

			printf("AF : %s\n", (host2->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

			for (int i = 0; host2->h_addr_list[i]; i++)
			{
				printf("ip addr : %s\n", inet_ntoa(*(IN_ADDR*)host2->h_addr_list[i]));
			}
		}
		else
		{
			cout << "다시 입력하세요." << endl;
		}
	}
	
	WSACleanup();

	return 0;
}