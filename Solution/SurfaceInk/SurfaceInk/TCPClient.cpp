#include "pch.h"
#include "TCPClient.h"


TCPClient::~TCPClient()
{
	CleanUp();
}

int TCPClient::StartTcpConnection(const char* ip)
{
	// Return value == 0  : connection succeed
	//		  value == -1 : socket initialization falied
	//		  value == -2 : invalid ipv4 address
	//		  value == -3 : connection falied

	// Initialize Windows socket.
	WSAData wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsok = WSAStartup(ver, &wsData);

	if (wsok != 0) {
		return -1;
	}

	// Create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}

	// Bind the socket to ip:port
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	int convertState = inet_pton(AF_INET, ip, &server.sin_addr);
	if (convertState == 0) {
		return -2;
	}

	// Connect to server
	connfd = connect(sockfd, (sockaddr*)&server, sizeof(server));
	if (connfd == SOCKET_ERROR) {
		closesocket(sockfd);
		return -3;
	}
	return 0;
}

void TCPClient::CleanUp()
{
	closesocket(sockfd);
	WSACleanup();
}
