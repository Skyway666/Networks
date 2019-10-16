#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		reportError("Error when initializing socket library");

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		reportError("Error while creating socket");
	// - Set address reuse
	int enable = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)))
		reportError("Error setting up socket to reuse IP and Port");
	// - Bind the socket to a local interface
	address.sin_family = AF_INET; //IPv4
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(listenSocket, (const sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
		reportError("Error while binding socket to local adress");
	// - Enter in listen mode

	if (listen(listenSocket, 5) == SOCKET_ERROR)
		reportError("Error while setting socket to listen (max 5)");
	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		for (auto message : messages)
			ImGui::Text("Message from %s: %s ", message.playerName.c_str(), message.message.c_str());


		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, InputMemoryStream & data)
{
	// Set the player name of the corresponding connected socket proxy

	ClientMessage clientMessage;
	data >> clientMessage;

	switch (clientMessage) {
		case ClientMessage::Hello:
		{
			std::string playerName;
			data >> playerName;
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {

					// Set player name
					connectedSocket.playerName = playerName;
					// Store welcome message in server
					std::string welcome_message = "\n'''\n" + playerName + " entered the chat!!\n" + "'''";
					messages.push_back(Message(welcome_message, "Server"));

					// Send welcome message to clients
					OutputMemoryStream out;
					out << ServerMessage::Welcome;
					out << welcome_message;
					sendPacketToAllUsers(connectedSocket.socket, out);
				}
			}
		}

		break;

		case ClientMessage::RegularMessage:
		{
			std::string message;
			data >> message;
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {
					// Receive message
					messages.push_back(Message(message, connectedSocket.playerName));

					OutputMemoryStream out;
					out << ServerMessage::UserMessage;
					out << connectedSocket.playerName;
					out << message;
					// Resend to all users
					sendPacketToAllUsers(connectedSocket.socket, out);
				}
			}
		}
		break;
	}

}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

void ModuleNetworkingServer::sendPacketToAllUsers(SOCKET socket, OutputMemoryStream & data) {
	for (auto &users : connectedSockets)
		if (users.socket != socket && !isListenSocket(users.socket))
			sendPacket(data, users.socket);
}

