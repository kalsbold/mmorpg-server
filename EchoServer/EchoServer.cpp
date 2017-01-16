// EchoServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <gisunnet/NetServer.h>

int main()
{
	using namespace gisunnet;

	auto net_server = NetServer::Create();
	net_server->Start(8413);

	std::string input;
	while (true)
	{
		std::getline(std::cin, input, '\n');
		if (std::strcmp(input.c_str(), "exit"))
			break;
	}
	
	net_server->Stop();

    return 0;
}

