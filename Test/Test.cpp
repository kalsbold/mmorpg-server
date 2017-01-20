// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <future>
#include <gisunnet/network/Server.h>
#include <gisunnet/network/Client.h>
#include <gisunnet/network/IoServicePool.h>

int main()
{
	using namespace gisunnet;

	Configuration config;
	config.io_service_pool = std::make_shared<IoServicePool>(4);
	auto server = Server::Create(config);

	server->RegisterSessionOpenedHandler([](auto& session)
	{
		std::cout << "Connect session id :" << session->ID() << "\n";
		string ip;
		uint16_t port;
		session->GetRemoteEndpoint(ip, port);
		std::cout << "Connect from :" << ip << ":" << port <<"\n";
	});

	server->RegisterSessionClosedHandler([](auto& session, auto& reason)
	{
		std::cout << "Close session id :" << session->ID() << "\n";
	});

	server->Start(8413);

	ClientConfiguration clientConfig;
	auto client = Client::Create(clientConfig);
	client->RegisterNetEventHandler([](const NetEventType& net_event)
	{
		if (net_event == NetEventType::Opened)
		{
			std::cout << "Connect success<<" << "\n";
		}
		else if (net_event == NetEventType::ConnectFailed)
		{
			std::cout << "Connect failed" << "\n";
		}
		else if (net_event == NetEventType::Closed)
		{
			std::cout << "Connect close" << "\n";
		}
	});

	client->Connect("127.0.0.1", "8413");
	while (true)
	{
		std::string str;
		std::cin >> str;
		auto buffer = std::make_shared<Buffer>(str.size());
		buffer->WriteBytes((uint8_t*)str.data(), str.size());
		client->Send(std::move(buffer));
	}

	return 0;
}

