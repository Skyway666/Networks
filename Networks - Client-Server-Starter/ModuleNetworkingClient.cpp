#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		reportError("Error when initializing socket library");

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		reportError("Error while creating socket");
	// - Create the remote address object
	address.sin_family = AF_INET; //IPv4
	address.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverAddressStr, &address.sin_addr);

	// - Connect to the remote address
	if (connect(clientSocket, (const sockaddr*)&address, sizeof(sockaddr_in)) == SOCKET_ERROR)
		reportError("Error while connecting to server");
	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(clientSocket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		// TODO(jesus): Send the player name to the server
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		state = ClientState::Logging;

		if (sendPacket(packet, clientSocket) == SOCKET_ERROR){
			reportError("Error while sending message from client");
			state = ClientState::Stopped;
			disconnect();
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		for (auto message : messages)
			ImGui::Text("Message from %s: %s ", message.playerName.c_str(), message.message.c_str());

		if (ImGui::Button("DISCONNECT")) {
			disconnect();
			state = ClientState::Stopped;
		}

		static char message[100];
		ImGui::InputText("Message Input", message, 100);

		if (ImGui::Button("Send")) {
			OutputMemoryStream packet;
			packet << ClientMessage::RegularMessage;
			std::string str_mssg = message;
			packet << str_mssg;

			if (sendPacket(packet, clientSocket) == SOCKET_ERROR) {
				reportError("Error while sending message from client");
				state = ClientState::Stopped;
				disconnect();
			}
			messages.push_back(Message(message, playerName));

		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, InputMemoryStream & data)
{
	ServerMessage serverMessage;
	data >> serverMessage;

	switch (serverMessage) {
	case ServerMessage::Welcome:
		std::string welcome_message;
		data >> welcome_message;

		messages.push_back(Message(welcome_message, "Server"));
		break;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

