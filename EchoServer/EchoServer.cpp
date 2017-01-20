// EchoServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <gisunnet/network/Server.h>
#include <gisunnet/network/IoServicePool.h>

int main()
{
	using namespace gisunnet;

	Configuration config;
	config.max_session_count = 5;
	auto server = Server::Create(config);

	server->RegisterSessionOpenedHandler([](auto& session)
	{
		std::cout << "Connect session id :" << session->ID() << "\n";
		string ip;
		uint16_t port;
		session->GetRemoteEndpoint(ip, port);
		std::cout << "Connect from :" << ip << ":" << port << "\n";
	});

	server->RegisterSessionClosedHandler([](auto& session, auto& reason)
	{
		std::cout << "Close session id :" << session->ID() << "\n";
	});

	server->Start(8413);

	std::string input;
	std::getline(std::cin, input, '\n');
	
	server->Stop();
	while (server->GetState() != Server::State::Stop)
	{
	}

    return 0;
}

