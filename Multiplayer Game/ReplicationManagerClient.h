#pragma once

#include"MemoryStream.h"
#include <vector>

class ReplicationManagerClient {
public:
	void read(InputMemoryStream& packet);
	void spawnObject(ObjectType tag);
};