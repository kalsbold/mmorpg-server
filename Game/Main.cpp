// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerSettings.h"
#include "GameServer.h"

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		if (argc < 2)
		{
			std::cerr << "Usage: cfg file path \n";
			return 1;
		}

		if (!ServerSettings::GetInstance().Load(argv[1]))
			return 0;

		auto server = std::make_shared<GameServer>();
		// ½ÃÀÛ
		server->Run();
		server->Wait();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() <<"\n";
	}

    return 0;
}

