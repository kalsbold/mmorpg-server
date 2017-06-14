// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Settings.h"
#include "ManagerServer.h"
#include "LoginServer.h"

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		/*if (argc < 2)
		{
			std::cerr << "Usage: cfg file path \n";
			return 1;
		}

		if (!Settings::GetInstance().Load(argv[1]))
			return 0;*/

		puts("Enterable commands:\n");
		puts("m: Start manager server.\n");
		puts("l: Start login server.\n");
		puts("q: Quit.\n");

		std::vector<Ptr<IServer>> server_list;
		std::string input;

		while (true)
		{
			std::cin >> input;

			if (input == "m")
			{
				if (!Settings::GetInstance().Load("manager.cfg"))
					return 0;

				auto server = std::make_shared<ManagerServer>();
				server->Run();
				server_list.push_back(server);
			}
			else if (input == "l")
			{
				if (!Settings::GetInstance().Load("login.cfg"))
					return 0;

				auto server = std::make_shared<LoginServer>();
				server->Run();
				server_list.push_back(server);
			}
			else if (input == "q")
			{
				break;
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() <<"\n";
	}

    return 0;
}

