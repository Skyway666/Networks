#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	RegularMessage,
	Bye
};

enum class ServerMessage
{
	Welcome,
	UserMessage,
	Disconnect
};

class Message {
public:
	Message(std::string message, std::string playerName, int type = 0): message(message), playerName(playerName), type(type){}

	std::string message;
	std::string playerName;
	int type; // ClientMessage, ServerMessage
};