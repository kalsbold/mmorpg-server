#pragma once

#include <map>
#include <mutex>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>
#include "Singleton.h"
#include "MySQL.h"
#include "game_message_generated.h"

using namespace gisunnet;

// 게임 서버.
class GameServer
{
public:
	using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
	using SessionClosedHandler = std::function<void(const Ptr<Session>&)>;
	using MessageHandler = std::function<void(const Ptr<Session>&, const Game::Protocol::NetMessage* net_message)>;

	GameServer();
	~GameServer();

	void Start();
	void Stop();

	void RegisterSessionOpenHandler(const SessionOpenedHandler& handler)
	{
		session_opened_handler_ = handler;
	}
	void RegisterSessionCloseHandler(const SessionClosedHandler& handler)
	{
		session_closed_handler_ = handler;
	}
	void RegisterMessageHandler(const Game::Protocol::Message& message_type, const MessageHandler& message_handler)
	{
		message_handler_map_.insert(std::make_pair(message_type, message_handler));
	}

	const Ptr<IoServicePool>& GetIoServicePool();
	const Ptr<MySQLPool>& GetDB();

private:
	void Initialize();
	void OnSessionOpen(const Ptr<Session>& session);
	void OnSessionClose(const Ptr<Session>& session, const CloseReason& reason);
	void OnSessionMessage(const Ptr<Session>& session, const uint8_t* buf, size_t bytes);

private:
	Ptr<IoServicePool> ios_pool_;
	Ptr<NetServer> net_server_;
	Ptr<MySQLPool> db_;

	std::mutex mutex_;
	SessionOpenedHandler session_opened_handler_;
	SessionClosedHandler session_closed_handler_;
	std::map<Game::Protocol::Message, MessageHandler> message_handler_map_;
};

inline GameServer::GameServer()
{
}

inline GameServer::~GameServer()
{
}

inline void GameServer::Initialize()
{
	// TO DO : 설정값을 파일에서 읽든가 외부에서 설정 가능하게
	size_t thread_count = 4;
	ios_pool_ = std::make_shared<IoServicePool>(thread_count);

	// Initialize NetServer
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

	// Initialize DB
	db_ = std::make_shared<MySQLPool>("127.0.0.1:8413", "nusigmik", "56561163", "Project_MMOG", 4);
}

inline void GameServer::OnSessionOpen(const Ptr<Session>& session)
{
	BOOST_LOG_TRIVIAL(info) << "On session open";

	if (session_opened_handler_)
		session_opened_handler_(session);
}

inline void GameServer::OnSessionClose(const Ptr<Session>& session, const CloseReason & reason)
{
	if (reason == CloseReason::ActiveClose)
	{
		BOOST_LOG_TRIVIAL(info) << "On session active close";
	}
	else if (reason == CloseReason::Disconnected)
	{
		BOOST_LOG_TRIVIAL(info) << "On session disconnected";
	}

	if (session_closed_handler_)
		session_closed_handler_(session);
}

inline void GameServer::OnSessionMessage(const Ptr<Session>& session, const uint8_t * buf, size_t bytes)
{
	//flatbuffers::Verifier verifier(buf, bytes);
	//if (!Game::Protocol::VerifyNetMessageBuffer(verifier))
	//{
	//	BOOST_LOG_TRIVIAL(info) << "Invalid NetMessage";
	//	return;
	//}
	const Game::Protocol::NetMessage* net_message = Game::Protocol::GetNetMessage(buf);
	if (net_message == nullptr)
	{
		BOOST_LOG_TRIVIAL(info) << "Invalid NetMessage";
		return;
	}

	try
	{
		auto handler = message_handler_map_.at(net_message->message_type());
		handler(session, net_message);
	}
	catch (const std::exception&)
	{
		BOOST_LOG_TRIVIAL(info) << "Invalid message_type ";
	}
}

inline void GameServer::Start()
{
	Initialize();

	uint16_t bind_port = 8413;
	net_server_->Start(bind_port);

	BOOST_LOG_TRIVIAL(info) << "Run Game Server";
	ios_pool_->Wait();
}

inline void GameServer::Stop()
{
	net_server_->Stop();
	
	// 종료 작업.


	ios_pool_->Stop();
	
	BOOST_LOG_TRIVIAL(info) << "Stop Game Server";
}

inline const Ptr<IoServicePool>& GameServer::GetIoServicePool()
{
	return ios_pool_;
}

inline const Ptr<MySQLPool>& GameServer::GetDB()
{
	return db_;
}
