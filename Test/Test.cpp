// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <future>
#include <gisunnet/IoServicePool.h>
#include <gisunnet/TcpServer.h>
#include <gisunnet/TcpClient.h>
#include <gisunnet/TcpTransport.h>

int main()
{
	using namespace gisunnet;

	auto ios_pool = std::make_shared<IoServicePool>(1);
	TcpServer server(ios_pool);

	server.ConnectHandler = [](TcpServer::TransportPtr& transport)
	{
		std::cout << "Connect from :" << transport->GetRemoteEndpoint() << "\n";
	};

	server.Listen("localhost", 8843);

	gisunnet::TcpClient client(ios_pool);
	client.ConnectHandler = [](const TcpClient::error_code& error, TcpClient::TransportPtr& transport)
	{
		if (!error)
		{
			std::cout << "Connect success to : " << transport->GetRemoteEndpoint() << "\n";
		}
		else
		{
			std::cout << "Connect failed : " << error.message() << "\n";
		}
	};

	client.Connect("192.168.219.193", "8843");

	ios_pool->Wait();

	return 0;
}

