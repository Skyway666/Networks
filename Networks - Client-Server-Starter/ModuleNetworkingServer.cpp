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


	// Initialize colors
	colors[0] = ImVec4((float)92/255, (float)50 / 255, (float)200 / 255, 1); //Purple
	colors[1] = ImVec4((float)108 / 255, (float)70 / 255, (float)117 / 255, 1); //Purple light
	colors[2] = ImVec4(0, 0, 1, 1); //Blue
	colors[3] = ImVec4(1, (float)128 / 255, 0, 1); //Orange
	colors[MAX_USERS - 1] = ImVec4(1, 1, 1, 1); //White

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

bool ModuleNetworkingServer::gui() {
	if (state != ServerState::Stopped) {
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets) {
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

		drawMessages();

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

	ClientMessage type;
	data >> type;

	switch (type) {
		case ClientMessage::Hello: // Notify all users of someone connected
		{
			std::string playerName;
			data >> playerName;
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {

					//Check stuff
					bool server_full = connectedSockets.size() - 1 >= MAX_USERS;
					bool used_name = usedName(playerName);
					bool server_name = playerName == "Server";

					if (server_full ||  used_name || server_name) {
						// Reject Player
						OutputMemoryStream out;
						out << ServerMessage::Unable; // Set client to disconnect

						UnableLog log = UnableLog::ServerFull;
						if (server_name) log = UnableLog::ServerName;
						else if (used_name) log = UnableLog::UsedName;
						out << log; // Tell client disconnection reason

						sendPacket(out, connectedSocket.socket);
						return; // Exit function
					}


					// Set player name and color
					connectedSocket.playerName = playerName;
					connectedSocket.color = colors[current_user_color++];
					// Store welcome message in server
					std::string welcome_message = "\n'''\n" + playerName + " entered the chat!!\n" + "'''";
					messages.push_back(Message(welcome_message, "Server", (int)type));

					// Send welcome message to clients
					OutputMemoryStream out;
					out << ServerMessage::Welcome;
					out << connectedSocket.color.x; out << connectedSocket.color.y; out << connectedSocket.color.z; out << connectedSocket.color.w; //TODO: Make function
					out << welcome_message;
					sendPacketToAllUsers(INVALID_SOCKET, out);
				}
			}
			break;
		}
		case ClientMessage::RegularMessage: // Notify all users someone sent a message
		{
			std::string message;
			data >> message;
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {
						// Store message
						messages.push_back(Message(message, connectedSocket.playerName, (int)type, connectedSocket.color));

						OutputMemoryStream out;
						out << ServerMessage::UserMessage;
						out << connectedSocket.playerName;
						out << connectedSocket.color.x; out << connectedSocket.color.y; out << connectedSocket.color.z; out << connectedSocket.color.w; //TODO: Make function
						out << message;
						// Resend to all users
						sendPacketToAllUsers(connectedSocket.socket, out);
				}
			}
			break;
		}

		case ClientMessage::SimonSays:
		{
			std::string message;
			data >> message;
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {

					// Resend Simon says
					OutputMemoryStream out;
					out << ServerMessage::SimonSays;
					out << connectedSocket.playerName;
					out << connectedSocket.color.x; out << connectedSocket.color.y; out << connectedSocket.color.z; out << connectedSocket.color.w; //TODO: Make function
					out << message;

					sendPacketToAllUsers(connectedSocket.socket, out);

					// Store message in server
					std::string simon_says_display = connectedSocket.playerName + " started a Simon Says! The word is: " + message;
					messages.push_back(Message(simon_says_display, connectedSocket.playerName, (int)ClientMessage::SimonSays, connectedSocket.color));
				}
			}
			break;
		}
		case ClientMessage::Bye: //Notify users sommeone disconnected
		{
			for (auto &connectedSocket : connectedSockets) {
				if (connectedSocket.socket == socket) {
					std::string disconnect = "\n'''\n" + connectedSocket.playerName + " left the chat\n'''";
					// Store message
					messages.push_back(Message(disconnect, "Server", (int)type));

					OutputMemoryStream out;
					out << ServerMessage::Disconnect;
					out << disconnect;
					// Resend to all users
					sendPacketToAllUsers(connectedSocket.socket, out);
				}
			}
	

			break;
		}
		
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

bool ModuleNetworkingServer::usedName(std::string name) {
	for (auto connectedSocket : connectedSockets)
		if (connectedSocket.playerName == name)
			return true;

	return false;
}

void ModuleNetworkingServer::drawMessages() {
	for (auto message : messages) {

		switch ((ClientMessage)message.type) {
		case ClientMessage::Hello:
		{
			ImGui::TextColored(ImVec4(0, 255, 0, 255), message.message.c_str());
			break;
		}
		case ClientMessage::RegularMessage:
		{
			ImGui::TextColored(message.color, "Message from %s: %s ", message.playerName.c_str(), message.message.c_str());
			break;
		}
		case ClientMessage::Bye:
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), message.message.c_str());
		}
		case ClientMessage::SimonSays:
		{
			ImGui::TextColored(message.color, message.message.c_str());
			break;
		}

		}
	}
}

