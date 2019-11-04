#include "Networks.h"
#include "ReplicationManagerServer.h"

void ReplicationManagerServer::create(uint32 networkID) {
	actions.push_back(ReplicationCommand(networkID, ReplicationAction::CREATE));
}

void ReplicationManagerServer::destroy(uint32 networkID) {
	actions.push_back(ReplicationCommand(networkID, ReplicationAction::DESTROY));
}

void ReplicationManagerServer::update(uint32 networkID) {
	actions.push_back(ReplicationCommand(networkID, ReplicationAction::UPDATE));
}

void ReplicationManagerServer::write(OutputMemoryStream & packet) {
	// Iterate actions and serialize object fields when needed.

}
