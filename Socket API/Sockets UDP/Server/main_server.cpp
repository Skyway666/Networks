#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

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

void server(int port)
{
	// TODO-1: Winsock init
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		printWSErrorAndExit("Error when initializing socket library");

	// TODO-2: Create socket (IPv4, datagrams, UDP
	SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket == INVALID_SOCKET)
		printWSErrorAndExit("Error while creating socket");

	// TODO-3: Force address reuse

	// TODO-3.1: Create an address object that can receive messages from any port
	sockaddr_in address;
	address.sin_family = AF_INET; //IPv4
	address.sin_port = htons(SERVER_PORT);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	// TODO-3.2: Enable the adress to reuse ports or IP that aren't properly closed
	int enable = 1;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)))
		printWSErrorAndExit("Error setting up socket to reuse IP and Port");


	// TODO-4: Bind to a local address
	if (bind(udp_socket, (const sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) 
		printWSErrorAndExit("Error while binding socket to local adress");

	
	while (true)
	{
		// TODO-5:
		// - Receive 'ping' packet from a remote host
		// - Answer with a 'pong' packet
		// - Control errors in both cases

		char* recieved_message = new char[100];
		sockaddr_in from;
		int size_of_address = sizeof(sockaddr_in);
		if (recvfrom(udp_socket, recieved_message, 100, 0, (sockaddr*)&from, &size_of_address) == SOCKET_ERROR)
			printWSErrorAndExit("Error while receiving message from server");
		else {
			fprintf(stderr, "message received by server: %s", recieved_message);
			Sleep(500);
		}

		const char* message = "pong";
		if (sendto(udp_socket, message, sizeof(message), 0, (const sockaddr*)&from, sizeof(from)) == SOCKET_ERROR)
			printWSErrorAndExit("Error while sending message from server");

	}

	// TODO-6: Close socket
	if (closesocket(udp_socket) == SOCKET_ERROR)
		printWSErrorAndExit("Error while closing socket");

	// TODO-7: Winsock shutdown
	if (WSACleanup() == SOCKET_ERROR)
		printWSErrorAndExit("Error while shutting down library");
}

int main(int argc, char **argv)
{
	server(SERVER_PORT);

	PAUSE_AND_EXIT();
}
