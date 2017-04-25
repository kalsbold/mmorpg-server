// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerConfig.h"
#include "GameServer.h"

using namespace std;
using namespace mmog;


int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		ServerConfig::GetInstance().Load("server.cfg");

		auto server = std::make_shared<GameServer>();
		// ½ÃÀÛ
		server->Run();
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << e.what();
	}

    return 0;
}

