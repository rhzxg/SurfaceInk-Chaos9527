#include "pch.h"
#include "TCPServer.h"

using namespace concurrency;


TCPServer::TCPServer()
{
    WSADATA wsaData;
    char name[255];
    char* ipaddr;
    PHOSTENT hostinfo;

    if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
    {
        if (gethostname(name, sizeof(name)) == 0)
        {
            if ((hostinfo = gethostbyname(name)) != NULL)
            {
                ipaddr = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
                ip = ipaddr;
            }
        }
        WSACleanup();
    }
}

TCPServer::~TCPServer()
{
	CleanUp();
}

//IAsyncAction^ TCPServer::StartTcpConnection()
//{
//	return create_async([this, first, last]
//		{
//			
//		});
//}

int TCPServer::StartTcpConnection()
{
	// Return value == -1 : socket error.

	// Initialize a windows socket
	//WSAData wsData;
	//WORD ver = MAKEWORD(2, 2);
	//int wsok = WSAStartup(ver, &wsData);

	if (wsok != 0) {
		return -1;
	}

	// Create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == INVALID_SOCKET) {
		return -1;
	}

	// Bind the socket to an ip:port
	//sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(9527);
	server.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sockfd, (sockaddr*)&server, sizeof(server));

	// Start listening
	listen(sockfd, 1);
	listening = true;
	// Wait for connection
	int client_sz = sizeof(client);
	connfd = accept(sockfd, (sockaddr*)&client, &client_sz);
	closesocket(sockfd);
	return 0;
}

void TCPServer::waitForConnection() {
	int client_sz = sizeof(client);
	connfd = accept(sockfd, (sockaddr*)&client, &client_sz);
	//closesocket(sockfd);
}

int TCPServer::recvFromClient() {
	//memset(buffer, '\0', MAXRECVSIZE);
	int n = recv(connfd, buffer, MAXRECVSIZE, 0);
	return n;
}

void TCPServer::CleanUp()
{
	closesocket(connfd);
	WSACleanup();
}