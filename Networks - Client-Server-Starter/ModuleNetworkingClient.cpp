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

		ImGui::Text("Logged as: %s", playerName.c_str());
		if (ImGui::Button("DISCONNECT")) 
			exitServer();
		

		drawMessages();
		ImGui::End();

		ImGui::Begin("Input Box");
		static char inp_message[100];
		ImGui::InputText("(Max 100 chars)", inp_message, 100);

		if (ImGui::Button("Send")) 
			sendServerMessage(inp_message, 100);

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, InputMemoryStream & data)
{
	ServerMessage type;
	data >> type;

	switch (type) {
	case ServerMessage::Unable: // Someone disconnected
	{
		std::string message;
		UnableLog error;
		data >> error;

		switch (error) {
			case UnableLog::ServerFull:
				ELOG("Coudn't connect, server full");
				break;
			case UnableLog::ServerName:
				ELOG("Coudn't connect, name 'Server' is not possible");
				break;
			case UnableLog::UsedName:
				ELOG("Coudn't connect, your name is being used");
			break;
		}


		disconnect();
		state = ClientState::Stopped;
		
		break;
	}
	case ServerMessage::Welcome: // Someone was connected
	{
		std::string welcome_message;
		ImVec4 color;
		data >> color.x; data >> color.y; data >> color.z; data >> color.w; //TODO: Make function
		if(!color_assigned){
			user_color = color;
			color_assigned = true;
		}
		data >> welcome_message;

		messages.push_back(Message(welcome_message, "Server", (int)type));
		break;
	}
	case ServerMessage::UserMessage: // Someone sent a message
	{
		std::string playerName;
		std::string message;
		ImVec4 color;
		data >> playerName;
		data >> color.x; data >> color.y; data >> color.z; data >> color.w; //TODO: Make function
		data >> message;


		messages.push_back(Message(message, playerName, (int)type, color));
		break;
	}
	case ServerMessage::Disconnect: // Someone disconnected
	{
		std::string message;
		data >> message;


		messages.push_back(Message(message, "Server", (int)type));
		break;
	}
	case ServerMessage::SimonSays:
	{
		// Code something so in 30 seconds, if the exact simon says message has not been sent the user gets disconnected

		// Receive data
		std::string playerName;
		std::string simon_says;
		ImVec4 color;
		data >> playerName;
		data >> color.x; data >> color.y; data >> color.z; data >> color.w; //TODO: Make function
		data >> simon_says;

		// Set up simon says display
		std::string simon_says_display = playerName + " started a Simon Says! The word is: " + simon_says;
		messages.push_back(Message(simon_says_display, playerName, (int)ClientMessage::SimonSays, color));

		//Start the minigame

		break;
	}

	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::sendServerMessage(char * message, int size) {
	// Set up packet
	OutputMemoryStream packet;
	packet << ClientMessage::RegularMessage;
	std::string str_mssg = message;
	packet << str_mssg;

	// Send packet and fill messages list
	if (sendPacket(packet, clientSocket) == SOCKET_ERROR) {
		reportError("Error while sending message from client");
		state = ClientState::Stopped;
		disconnect();
	}
	messages.push_back(Message(message, playerName, (int)ServerMessage::UserMessage, user_color));

	// Clean message buffer
	for (int i = 0; i < size; i++)
		message[i] = '\0';
}

void ModuleNetworkingClient::exitServer() {
	OutputMemoryStream out;
	out << ClientMessage::Bye;
	sendPacket(out, clientSocket);
	disconnect();
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::drawMessages() {
	for (auto message : messages) {

		switch ((ServerMessage)message.type) {
		case ServerMessage::Welcome:
		{
			ImGui::TextColored(ImVec4(0, 255, 0, 255), message.message.c_str());
			break;
		}
		case ServerMessage::UserMessage:
		{
			ImGui::TextColored(message.color, "Message from %s: %s ", message.playerName.c_str(), message.message.c_str());
			break;
		}
		case ServerMessage::Disconnect:
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), message.message.c_str());
			break;
		}
		case ServerMessage::SimonSays:
		{
			ImGui::TextColored(message.color, message.message.c_str());
			break;
		}

		}
	}
}

