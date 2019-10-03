#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define LISTEN_PORT 8888

#define PAUSE_AND_EXIT() system("pause"); exit(-1)

void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	PAUSE_AND_EXIT();
}

void server(int port)
{
	// TODO-1: Winsock init
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		printWSErrorAndExit("Error when initializing socket library");


	// TODO-2: Create socket (IPv4, stream, TCP)
	SOCKET tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == INVALID_SOCKET)
		printWSErrorAndExit("Error while creating socket");

	// TODO-3: Configure socket for address reuse
	int enable = 1;
	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)))
		printWSErrorAndExit("Error setting up socket to reuse IP and Port");


	// TODO-4: Create an address object with any local address
	sockaddr_in address;
	address.sin_family = AF_INET; //IPv4
	address.sin_port = htons(LISTEN_PORT);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	// TODO-5: Bind socket to the local address
	if (bind(tcp_socket, (const sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
		printWSErrorAndExit("Error while binding socket to local adress");

	// TODO-6: Make the socket enter into listen mode
	if (listen(tcp_socket, 5) == SOCKET_ERROR) 
		printWSErrorAndExit("Error while setting socket to listen (max 5)");
	

	// TODO-7: Accept a new incoming connection from a remote host
	// Note that once a new connection is accepted, we will have
	// a new socket directly connected to the remote host.
	int size_of_address = sizeof(sockaddr_in);
	SOCKET connected_tcp_socket = accept(tcp_socket, (sockaddr*)&address, &size_of_address);
	if(connected_tcp_socket == SOCKET_ERROR)
		printWSErrorAndExit("Error while setting tcp socket to accept");


	while (true)
	{
		// TODO-8:
		// - Wait a 'ping' packet from the client
		// - Send a 'pong' packet to the client
		// - Control errors in both cases

		char recieved_message[100];
		if (recv(connected_tcp_socket, recieved_message, 100, 0) == SOCKET_ERROR)
			printWSErrorAndExit("Error while receiving message from client");
		else {
			fprintf(stderr, "message received by server: %s", recieved_message);
			Sleep(500);
		}

		const char* message = "pong";
		if (send(connected_tcp_socket, message, sizeof(message), 0) == SOCKET_ERROR)
			printWSErrorAndExit("Error while sending message from server");

	}

	// TODO-9: Close socket
	if (closesocket(tcp_socket) == SOCKET_ERROR)
		printWSErrorAndExit("Error while closing socket");

	// TODO-10: Winsock shutdown
	if (WSACleanup() == SOCKET_ERROR)
		printWSErrorAndExit("Error while shutting down library");
}

int main(int argc, char **argv)
{
	server(LISTEN_PORT);

	PAUSE_AND_EXIT();
}
