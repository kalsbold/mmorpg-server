#pragma once
#include <gisunnet/gisunnet.h>

using namespace gisunnet;

class GameServer
{
public:
	GameServer();
	~GameServer();

	void Run();
	void Stop();

private:
	Ptr<IoServicePool> ios_pool_;
	Ptr<NetServer> net_server_;
};

GameServer::GameServer()
{
}

GameServer::~GameServer()
{
	Stop();
}

void GameServer::Run()
{
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

	ios_pool_ = ios_pool;
	net_server_ = server;

	BOOST_LOG_TRIVIAL(info) << "Run Game Server";
}

void GameServer::Stop()
{
	net_server_->Stop();
	
	// 종료 작업.


	ios_pool_->Stop();
	ios_pool_->Wait();
	
	BOOST_LOG_TRIVIAL(info) << "Stop Game Server";
}