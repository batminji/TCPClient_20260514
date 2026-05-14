#include "stdafx.h"

int PlayerX = 0;
int PlayerY = 0;

void ReceiveThread(SOCKET serverSocket);

void Gotoxy(int x, int y)
{
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int main()
{
	srand((unsigned int)time(nullptr));
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 1;
	}

	hostent* HostInfo = gethostbyname("login.calculate.edu");

	char ServerIP[1024] = { 0, };
	IN_ADDR Addr;
	Addr.s_addr = *(ULONG*)*HostInfo->h_addr_list;
	sprintf_s(ServerIP, "%s", inet_ntoa(Addr));
	printf("%s\n", ServerIP);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		std::cout << "Listen Socket Error" << std::endl;
		exit(-1);
	}

	SOCKADDR_IN ServerSocketAddr;
	memset(&ServerSocketAddr, 0, sizeof(ServerSocketAddr));
	ServerSocketAddr.sin_family = AF_INET;
	if (!inet_pton(AF_INET, ServerIP, (PVOID)&ServerSocketAddr.sin_addr.s_addr))
	{
		printf("inet_pton Error\n");
		exit(-1);
	}
	ServerSocketAddr.sin_port = htons(SERVER_PORT);

	int retval;
	retval = connect(ServerSocket, (SOCKADDR*)&ServerSocketAddr, sizeof(ServerSocketAddr));
	if (retval == SOCKET_ERROR)
	{
		printf("Connect Error\n");
		exit(-1);
	}

	std::thread RecvWorker(ReceiveThread, ServerSocket);
	RecvWorker.detach();

	while (true)
	{
		if (_kbhit())
		{
			char Key = _getch();
			char Dir = ' ';

			if (Key == 'w' || Key == 'W')
			{
				Dir = 'W';
			}
			else if (Key == 's' || Key == 'S')
			{
				Dir = 'S';
			}
			else if (Key == 'a' || Key == 'A')
			{
				Dir = 'A';
			}
			else if (Key == 'd' || Key == 'D')
			{
				Dir = 'D';
			}

			if (Dir != ' ')
			{
				PacketHeader SendHeader;
				SendHeader.Size = htons(sizeof(MoveData));
				SendHeader.Code = htons(static_cast<unsigned short>(PacketType::Move));

				MoveData SendData;
				SendData.Dir = Dir;

				send(ServerSocket, (char*)&SendHeader, sizeof(SendHeader), 0);
				send(ServerSocket, (char*)&SendData, sizeof(SendData), 0);
			}
		}

		Gotoxy(0, 0);
		printf("Player X: %d, Player Y: %d\n", PlayerX, PlayerY);
	}

	closesocket(ServerSocket);

	WSACleanup();
}

void ReceiveThread(SOCKET serverSocket)
{
	while (true)
	{
		PacketHeader RecvHeader;
		int retval = recv(serverSocket, (char*)&RecvHeader, sizeof(RecvHeader), MSG_WAITALL);
		if (retval <= 0) {
			printf("Sever Connect Header\n");
			break;
		}

		unsigned short Size = ntohs(RecvHeader.Size);
		unsigned short Code = ntohs(RecvHeader.Code);

		switch (static_cast<PacketType>(Code))
		{
		case PacketType::Position:
		{
			PositionData PosData;
			retval = recv(serverSocket, (char*)&PosData, Size, MSG_WAITALL);
			if (retval <= 0) break;

			PlayerX = ntohl(PosData.X);
			PlayerY = ntohl(PosData.Y);
		}
		break;
		default:
			char dummy[1024];
			recv(serverSocket, dummy, Size, MSG_WAITALL);
			break;
		}
	}
}