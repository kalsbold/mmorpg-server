// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <gisunnet/gisunnet.h>

void RunGameServer()
{
	namespace asio = boost::asio;
	using namespace gisunnet;
	
	BOOST_LOG_TRIVIAL(info) << "Run Game Server";

	size_t thread_count = 4;
	auto ios_pool = std::make_shared<IoServicePool>(thread_count);
	
	Configuration config;
	config.io_service_pool = ios_pool;
	config.max_session_count = 1000;
	config.max_receive_buffer_size = 4 * 1024;
	config.min_receive_size = 256;
	config.no_delay = true;

	Ptr<NetServer> server = NetServer::Create(config);
	server->RegisterSessionOpenedHandler([](const Ptr<Session>& session) {
		BOOST_LOG_TRIVIAL(info) << "On Session Open";
	});
	server->RegisterSessionClosedHandler([](const Ptr<Session>& session, const CloseReason& reason) {
		BOOST_LOG_TRIVIAL(info) << "On Session Close";
	});
	server->RegisterMessageHandler([](const auto& session, const uint8_t* buf, size_t bytes) {
		BOOST_LOG_TRIVIAL(info) << "On Session Message";
	});

	uint16_t bind_port = 8088;
	server->Start(bind_port);

	// 종료 signal
	/*asio::signal_set signals(ios_pool->PickIoService(), SIGINT, SIGTERM);
	signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
		if (!error)
		{
			LOG("Stop Server");
			server->Stop();
			LOG("Stop IoServicePool");
			ios_pool->Stop();
		}
	});*/

	while (auto ch = getchar())
	{
		if (ch == 'q')
		{
			server->Stop();
			ios_pool->Stop();
			break;
		}
	}

	// IoServicePool 이 정지될때 까지 기다린다.
	ios_pool->Wait();

	// 종료 작업.
	BOOST_LOG_TRIVIAL(info) << "Stop Game Server";
}

int main()
{
	RunGameServer();

    return 0;
}

