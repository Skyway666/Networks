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
	Disconnect,
	Unable,
	SimonSays
};

enum class UnableLog {
	ServerFull,
	UsedName,
	ServerName
};

class Message {
public:
	Message(std::string message, std::string playerName, int type = 0, ImVec4 color = ImVec4(0,0,0,255)): 
	message(message), playerName(playerName), type(type), color(color){}

	std::string message;
	std::string playerName;
	int type; // ClientMessage, ServerMessage
	ImVec4 color;
};