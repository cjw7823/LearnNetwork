#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

unsigned WINAPI ThreadIncrease(void* arg);
unsigned WINAPI ThreadDecrease(void* arg);

int main()
{
	SOCKET ServerSocket;
	SOCKET ClientSocket;

	SOCKADDR_IN ServerAddr;
	SOCKADDR_IN ClientAddr;

	fd_set Reads;
	fd_set Copys;


	WSAData wsaData;

	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup");
		exit(-1);
	}

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("socket");
		exit(-1);
	}

	FD_ZERO(&Reads);
	FD_SET(ServerSocket, &Reads);

	memset(&ServerAddr, 0, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = PF_INET; //IP V4
	ServerAddr.sin_port = htons(12345);
	ServerAddr.sin_addr.s_addr = INADDR_ANY; //아무거나

	bind(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	listen(ServerSocket, 5);

	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	printf("Server\n");

	while (1)
	{
		Copys = Reads;

		int fd_num = select(0, &Copys, 0, 0, &timeout);
		if (fd_num == SOCKET_ERROR)
		{
			break;
		}

		if (fd_num == 0)
		{
			continue;
			//다른 서버 로직
		}

		for (unsigned int i = 0; i < Reads.fd_count; ++i)
		{
			//이벤트가 일어나면 처리
			if (FD_ISSET(Reads.fd_array[i], &Copys))
			{
				if (Reads.fd_array[i] == ServerSocket)
				{
					int ClientAddrLength = sizeof(ClientAddr);
					ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddr, &ClientAddrLength);
					FD_SET(ClientSocket, &Reads);
					printf("connection client : %d\n", ClientSocket);
				}
				else
				{
					//클라이언트 데이터 처리
					char Buffer[1024] = { 0, };
					int RecvLength = 0;
					RecvLength = recv(Reads.fd_array[i], Buffer, sizeof(Buffer), 0);
					if (RecvLength == 0)
					{
						//close connection
						FD_CLR(Reads.fd_array[i], &Reads);
						closesocket(Reads.fd_array[i]);
						printf("close connection : %d\n", Reads.fd_array[i]);
					}
					else
					{
						for (unsigned int j = 0; j < Reads.fd_count; j++)
						{
							if (Reads.fd_array[j] != ServerSocket)
							{
								if (Reads.fd_array[j] != Reads.fd_array[i])
								{
									send(Reads.fd_array[j], Buffer, RecvLength, 0);
								}
							}
						}

					}
				}
			}
		}
	}

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}