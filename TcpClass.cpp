#include "stdafx.h"
#include "TcpClass.h"


TcpClass::TcpClass()
{
}

bool TcpClass::InitializeTcpClass()
{
	if (!Connected)
	{

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return 0;
		}


		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		std::cout << "Init tcp" << std::endl;
		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return 0;
		}



		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return false;
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			WSACleanup();
			return false;
		}
	
		Connected = true;
		return true;
	}
	return true;
}

void TcpClass::UpdateActorList(std::vector<Actor*> actors)
{
	this->actorsList = actors;
}

void TcpClass::PrepareTransmission()
{
	Message = "";

	for (size_t i = 0; i < actorsList.size(); i++)
	{
		if (i == actorsList.size() -1)
		{
			Message += actorsList[i]->ObjectTransmissionString();
		}
		else
		{
			Message += actorsList[i]->ObjectTransmissionString() + "::::";
		}
	}

	if (Message.size() < 10000)
	{
		sendbuf = Message.c_str();
	}
	else
	{
		MessageBox(NULL, L"Cannot send", L"MESSAGE TOO LONG", MB_OK);
	}
	int asd = 0;
}

void TcpClass::Send()
{
	// Send an initial buffer






	iResult = send(ConnectSocket, Message.c_str(), Message.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
}


void TcpClass::Release()
{
	closesocket(ConnectSocket);
	WSACleanup();
}

TcpClass::~TcpClass()
{
}
