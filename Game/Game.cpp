// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GameServer.h"

int main()
{
	auto& server = GameServer::GetInstance();
	server.Initialize();
	server.Run(8084);

    return 0;
}

