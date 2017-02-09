#pragma once

#include <map>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>
#include "Singleton.h"

using namespace gisunnet;

// 게임 서버.
class GameServer : public Singleton<GameServer>
{
public:
	void Initialize();
	void Run(uint16_t bind_port);
	void Stop();
	const Ptr<IoServicePool>& GetIoServicePool()
	{
		return ios_pool_;
	}

private:
	Ptr<IoServicePool> ios_pool_;
	Ptr<NetServer> net_server_;

	void OnSessionOpen(const Ptr<Session>& session)
	{
		BOOST_LOG_TRIVIAL(info) << "On session open";
	}

	void OnSessionClose(const Ptr<Session>& session, const CloseReason& reason)
	{
		if (reason == CloseReason::ActiveClose)
		{
			BOOST_LOG_TRIVIAL(info) << "On session active close";
		}
		else if (reason == CloseReason::Disconnected)
		{
			BOOST_LOG_TRIVIAL(info) << "On session disconnected";
		}
	}

	void OnSessionMessage(const Ptr<Session>& session, const uint8_t* buf, size_t bytes)
	{
		BOOST_LOG_TRIVIAL(info) << "On session message";
	}
};

void GameServer::Initialize()
{
	size_t thread_count = 4;
	ios_pool_ = std::make_shared<IoServicePool>(thread_count);

	Configuration config;
	config.io_service_pool = ios_pool_;
	config.max_session_count = 1000;
	config.max_receive_buffer_size = 4 * 1024;
	config.min_receive_size = 256;
	config.no_delay = true;

	net_server_ = NetServer::Create(config);

	net_server_->RegisterSessionOpenedHandler([this](const Ptr<Session>& session) {
		OnSessionOpen(session);
	});

	net_server_->RegisterSessionClosedHandler([this](const Ptr<Session>& session, const CloseReason& reason) {
		OnSessionClose(session, reason);
	});

	net_server_->RegisterMessageHandler([this](const Ptr<Session>& session, const uint8_t* buf, size_t bytes) {
		OnSessionMessage(session, buf, bytes);
	});

	// Init DB
}

void GameServer::Run(uint16_t bind_port)
{
	if (ios_pool_ == nullptr || net_server_ == nullptr)
	{
		throw std::logic_error("Initialize required");
	}

	net_server_->Start(bind_port);

	BOOST_LOG_TRIVIAL(info) << "Run Game Server";
	ios_pool_->Wait();
}

void GameServer::Stop()
{
	net_server_->Stop();
	
	// 종료 작업.


	ios_pool_->Stop();
	
	BOOST_LOG_TRIVIAL(info) << "Stop Game Server";
}