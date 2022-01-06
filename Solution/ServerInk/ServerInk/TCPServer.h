#pragma once

#include <WS2tcpip.h>
#include <string>
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable:4996)
#define MAXRECVSIZE 4194304
#include <ppltasks.h>


class TCPServer
{
public:
	TCPServer();
	~TCPServer();
	int StartTcpConnection();
	void waitForConnection();
	int recvFromClient();
	void CleanUp();
	std::string ip;
	SOCKET sockfd;
	SOCKET connfd;
	sockaddr_in client;

	WSAData wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsok = WSAStartup(ver, &wsData);
	
	sockaddr_in server;

	bool listening = false;

	
	// Up to 4Mb to receive.
	char buffer[MAXRECVSIZE];
};

