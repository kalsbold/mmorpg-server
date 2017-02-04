#pragma once
#include <gisunnet/network/Server.h>

using namespace gisunnet;

class GameServer
{
public:
	GameServer();
	~GameServer();

private:

	Ptr<Server> tcp_server;
};

GameServer::GameServer()
{
}

GameServer::~GameServer()
{
}