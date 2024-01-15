#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main2()
{

	SOCKADDR_IN Addr;

	WSAData wsaData;

	struct hostent* host;

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup");
		exit(-1);
	}

	host = gethostbyname("google.com");

	for (int i = 0; host->h_aliases[i]; i++)
	{
		printf("alias : %s\n", host->h_aliases[i]);
	}

	printf("AF : %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

	for (int i = 0; host->h_addr_list[i]; i++)
	{
		printf("ip addr : %s\n", inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]));
	}

	printf("------------------------------------------------------\n");

	memset(&Addr, 0, sizeof(SOCKADDR_IN));
	Addr.sin_addr.s_addr = inet_addr("172.217.175.14");

	host = nullptr;

	host = gethostbyaddr((char*)&Addr.sin_addr, 4, AF_INET);

	for (int i = 0; host->h_aliases[i]; i++)
	{
		printf("alias : %s\n", host->h_aliases[i]);
	}

	printf("AF : %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

	for (int i = 0; host->h_addr_list[i]; i++)
	{
		printf("ip addr : %s\n", inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]));
	}


	WSACleanup();

	return 0;
}