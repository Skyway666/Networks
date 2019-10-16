#include "Networks.h"
#include "ModuleNetworking.h"

#include <list>

static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
		messages.clear();
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];


	// TODO(jesus): select those sockets that have a read operation available

	// Initialize socket set
	fd_set readSet;
	FD_ZERO(&readSet);
	// Fill socket set with the sockets in the list
	for (auto s:sockets) {
		FD_SET(s, &readSet);
	}

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if(select(0, &readSet, nullptr, nullptr, &timeout) == SOCKET_ERROR)
		reportError("Error when selecting sockets");

	
	std::list<SOCKET> disconnectedSockets;

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).

	for (auto s : sockets) {
		if (FD_ISSET(s, &readSet)) {
			if (isListenSocket(s)) { // Listener, time to connect
				int size_of_address = sizeof(address);
				SOCKET connected_socket = accept(s, (sockaddr*)&address, &size_of_address);
				if(connected_socket == SOCKET_ERROR)
					reportError("Error while connecting to client");
				else {
					onSocketConnected(connected_socket, address);
					addSocket(connected_socket);
				}

			}
			else { // Data, Time to receive

				// TODO(jesus): handle disconnections. Remember that a socket has been
				// disconnected from its remote end either when recv() returned 0,
				// or when it generated some errors such as ECONNRESET.
				// Communicate detected disconnections to the subclass using the callback
				// onSocketDisconnected().

				InputMemoryStream packet;
				auto received_bytes = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0);

				// Error
				if (received_bytes == SOCKET_ERROR)
					reportError("Error while receiving message from server");
				// Remote socket notifies disconection
				else if (received_bytes == 0 || received_bytes == ECONNRESET) {
					onSocketDisconnected(s);
					disconnectedSockets.push_back(s);
				}
				// Message received correctly
				else {
					packet.SetSize((uint32)received_bytes);
					onSocketReceivedData(s, packet);
				}
					
			}

		}
	}
	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.
	for (auto disc_sock : disconnectedSockets)
		sockets.erase(std::remove(sockets.begin(), sockets.end(), disc_sock), sockets.end());
	

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream & packet, SOCKET socket) {

	if (send(socket, packet.GetBufferPtr(), packet.GetSize(), 0) == SOCKET_ERROR) {
		reportError("Send");
		return false;
	}
	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}