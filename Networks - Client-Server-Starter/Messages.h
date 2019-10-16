#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	RegularMessage
};

enum class ServerMessage
{
	Welcome,
	UserMessage
};

class Message {
public:
	Message(std::string message, std::string playerName): message(message), playerName(playerName){}

	std::string message;
	std::string playerName;
};