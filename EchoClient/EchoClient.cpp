// EchoClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <gisunnet/IoServicePool.h>
#include <gisunnet/TcpClient.h>
#include <gisunnet/TcpTransport.h>

int main()
{
	using namespace gisunnet;

	auto ios_pool = std::make_shared<IoServicePool>(2);
	TcpClient client(ios_pool);

	client.ConnectHandler = [](const TcpClient::error_code& error, TcpClient::TransportPtr& transport)
	{
		if (error)
		{
			std::cout << " Connect Error : " << error.message() << "\n";
			return;
		}
		std::cout << "Connect success. -> " << transport->GetRemoteEndpoint() << "\n";
	};

	client.Connect("127.0.0.1", "8413");

	ios_pool->Wait();

    return 0;
}
