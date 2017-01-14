// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <future>
#include <gisunnet/IoServicePool.h>
#include <gisunnet/TcpServer.h>
#include <gisunnet/TcpTransport.h>

int main()
{
	auto ios_pool = std::make_shared<gisunnet::IoServicePool>(4);
	gisunnet::TcpServer server(ios_pool);

	auto f = std::async(std::launch::async, [&]
	{
		server.AcceptedCallback = [](gisunnet::TcpServer::TransportPtr& transport)
		{
			std::cout << transport->GetRemoteEndpoint() << "\n";
		};

		server.Start(8843, boost::asio::ip::tcp::v6());

		ios_pool->Wait();
	});

	f.wait();

	return 0;
}

