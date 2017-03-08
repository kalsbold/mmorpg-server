#pragma once

#include <map>
#include <mutex>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>
#include "MySQL.h"
#include "game_message_generated.h"

#include "GameSession.h"

using namespace gisunnet;

class ServerConfig
{
public:
	string bind_address = "0.0.0.0";
	int bind_port;
	size_t thread_count = 1;
	size_t max_session_count = 1000;
};

// 게임 서버.
class GameServer
{
public:
	using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
	using SessionClosedHandler = std::function<void(const Ptr<Session>&)>;
	using MessageHandler = std::function<void(const Ptr<Session>&, const Game::Protocol::NetMessage* net_message)>;

	GameServer(const ServerConfig& config)
		: config_(config)
	{
	}
	~GameServer()
	{
	}

	void Run();
	void Stop();

	const Ptr<IoServicePool>& GetIoServicePool()
	{
		return ios_pool_;
	}
	const Ptr<MySQLPool>& GetDB()
	{
		return db_;
	}

private:
	void RegisterSessionOpenHandler(const SessionOpenedHandler& handler)
	{
		session_opened_handler_ = handler;
	}
	void RegisterSessionCloseHandler(const SessionClosedHandler& handler)
	{
		session_closed_handler_ = handler;
	}
	void RegisterMessageHandler(const Game::Protocol::MessageT& message_type, const MessageHandler& message_handler)
	{
		message_handler_map_.insert(std::make_pair(message_type, message_handler));
	}

	void Initialize();
	void InitializeHandlers();

	// Handler ===============================================================
	void OnSessionOpen(const Ptr<Session>& session);
	void OnSessionClose(const Ptr<Session>& session);
	
	// Message Handler =======================================================
	void OnJoinRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void JoinSuccessReply(const Ptr<Session>& session);
	void JoinFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);

	void OnLoginRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void LoginSuccessReply(const Ptr<Session>& session, const std::string& session_id);
	void LoginFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);

private:
	Ptr<IoServicePool> ios_pool_;
	Ptr<NetServer> net_server_;
	Ptr<MySQLPool> db_;

	std::mutex mutex_;
	ServerConfig config_;
	SessionOpenedHandler session_opened_handler_;
	SessionClosedHandler session_closed_handler_;
	std::map<Game::Protocol::MessageT, MessageHandler> message_handler_map_;

	// 유저 관리
	std::map<uuid,Ptr<GameSession>> game_session_map_;
};
