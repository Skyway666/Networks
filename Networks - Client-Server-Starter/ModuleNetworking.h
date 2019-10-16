#pragma once


class ModuleNetworking : public Module
{
private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool init() override;

	bool preUpdate() override;

	bool cleanUp() override;



	//////////////////////////////////////////////////////////////////////
	// Socket event callbacks
	//////////////////////////////////////////////////////////////////////

	virtual bool isListenSocket(SOCKET socket) const { return false; }

	virtual void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) { }

	virtual void onSocketReceivedData(SOCKET s, InputMemoryStream & data) = 0;

	virtual void onSocketDisconnected(SOCKET s) = 0;

	

protected:

	std::vector<SOCKET> sockets;

	std::vector<Message> messages;

	sockaddr_in address; // For the server is any client, for the client is the server

	void addSocket(SOCKET socket);

	void disconnect();

	static void reportError(const char *message);

	static bool sendPacket(const OutputMemoryStream & packet, SOCKET socket);

};
