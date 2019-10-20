#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, InputMemoryStream & data) override;

	void onSocketDisconnected(SOCKET socket) override;

	void sendServerMessage(char* message, int size);

	void exitServer();
	
	void drawMessages();



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging,
		SimonSays
	};

	ClientState state = ClientState::Stopped;

	SOCKET clientSocket = (SOCKET)(~0);

	std::string playerName;

	ImVec4 user_color;
	bool color_assigned = false;


	// Simon says badly coded stuff
	int simon_says_start = 0;
	int simon_says_duration = 10; // Seconds
	std::string simon_says_keyword;

	void startSimonSays(std::string keyword);
	void stopSimonSays();
};

