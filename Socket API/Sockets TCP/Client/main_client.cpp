#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"

#define SERVER_PORT 8888

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

void client(const char *serverAddrStr, int port)
{
	// TODO-0:  Wait for the server to be ready
	Sleep(1000); 

	// TODO-1: Winsock init
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		printWSErrorAndExit("Error when initializing socket library");

	// TODO-2: Create socket (IPv4, stream, TCP)
	SOCKET tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == INVALID_SOCKET)
		printWSErrorAndExit("Error while creating socket");

	// TODO-3: Create an address object with the server address
	sockaddr_in address;
	address.sin_family = AF_INET; //IPv4
	address.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr);

	// TODO-4: Connect to server
	if(connect(tcp_socket, (const sockaddr*) &address, sizeof(sockaddr_in)) == SOCKET_ERROR)
		printWSErrorAndExit("Error while connecting to server");

	for (int i = 0; i < 5; ++i)
	{
		// TODO-5:
		// - Send a 'ping' packet to the server
		// - Receive 'pong' packet from the server
		// - Control errors in both cases
		// - Control graceful disconnection from the server (recv receiving 0 bytes)
		const char* message = "ping";
		if(send(tcp_socket, message, sizeof(message), 0) == SOCKET_ERROR)
			printWSErrorAndExit("Error while sending message from client");


		char* recieved_message = new char[100];
		if(recv(tcp_socket, recieved_message, 100, 0) == SOCKET_ERROR)
			printWSErrorAndExit("Error while receiving message from server");


	}

	// TODO-5: Close socket
	if (closesocket(tcp_socket) == SOCKET_ERROR)
		printWSErrorAndExit("Error while closing socket");

	// TODO-6: Winsock shutdown
	if (WSACleanup() == SOCKET_ERROR)
		printWSErrorAndExit("Error while shutting down library");
}

int main(int argc, char **argv)
{
	client(SERVER_ADDRESS, SERVER_PORT);

	PAUSE_AND_EXIT();
}
