// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerConfig.h"
#include "GameServer.h"

using namespace mmog;

void RunServer()
{
	// 게임서버 설정
	ServerConfig& config = ServerConfig::GetInstance();
	config.bind_port = 8088;
	config.thread_count = 4;
	config.db_host = "127.0.0.1:3306";
	config.db_user = "nusigmik";
	config.db_password = "56561163";
	config.db_schema = "Project_MMOG";
	config.db_connection_pool = 4;

	try
	{
		auto server = std::make_shared<GameServer>();
		server->Run();
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << e.what();
	}
}

int main()
{
	RunServer();

    return 0;
}

