#pragma once
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#define MAXSENDSIZE 4194304


class TCPClient
{
public:
	~TCPClient();
	int StartTcpConnection(const char* ip);
	void CleanUp();

	const int port = 9527;
	SOCKET sockfd;
	SOCKET connfd;

	char buffer[MAXSENDSIZE];
};