#pragma once

#ifndef TCP_CLASS
#define TCP_CLASS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "Actor.h"



// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")





class TcpClass
{

private:

#define DEFAULT_BUFLEN 10000
#define DEFAULT_PORT "13"


	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	const char *sendbuf = "";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	PCSTR argv[128];
	std::string Message;


	std::vector<Actor*> actorsList;

public:
	TcpClass();

	bool Connected = false;
	bool InitializeTcpClass();

	void UpdateActorList(std::vector<Actor*> actors);

	void PrepareTransmission();

	void Send();


	void Release();

	~TcpClass();
};

#endif