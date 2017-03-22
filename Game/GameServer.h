#pragma once

#include <map>
#include <mutex>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>
#include "MySQL.h"
#include "game_message_generated.h"

#include "GameUser.h"

using namespace gisunnet;

enum class ClassType : int
{
	Knight = 0,
	Archer = 1,
	Mage = 2,
};

struct HeroSimpleData
{
	int id;
	std::string name;
	ClassType class_type;
	int level;
};

// ���� ����.
class GameServer
{
public:
	using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
	using SessionClosedHandler = std::function<void(const Ptr<Session>&)>;
	using MessageHandler = std::function<void(const Ptr<Session>&, const Game::Protocol::NetMessage* net_message)>;

	GameServer()
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

	void InitializeHandlers();
	
	// Handler ===============================================================
	void OnSessionOpen(const Ptr<Session>& session);
	void OnSessionClose(const Ptr<Session>& session);
	
	// Message Handler =======================================================
	void OnJoinRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	// Join
	void JoinSuccessReply(const Ptr<Session>& session);
	void JoinFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);
	// Login
	void OnLoginRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void LoginSuccessReply(const Ptr<Session>& session, const std::string& session_id);
	void LoginFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);
	// Create Hero
	void OnCreateHeroRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void CreateHeroSuccessReply(const Ptr<Session>& session, int id, const char* name, int class_type, int level);
	void CreateHeroFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);
	// Hero List
	void OnHeroListRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void HeroListReply(const Ptr<Session>& session, const std::vector<HeroSimpleData>& hero_list);
	// Delete Hero
	void OnDeleteHeroRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage* net_message);
	void DeleteHeroSuccessReply(const Ptr<Session>& session, int hero_id);
	void DeleteHeroFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code);

private:
	const Ptr<GameUser> FindGameUserSession(const uuid& session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = game_user_session_map_.find(session_id);
		if (iter == game_user_session_map_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	// Account ID�� ã�´�
	const Ptr<GameUser> FindGameUserSession(int account_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = std::find_if(game_user_session_map_.begin(), game_user_session_map_.end(), [account_id](const std::pair<uuid, Ptr<GameUser>>& pair)
		{
			auto info = pair.second->GetAccountInfo();
			return info.id == account_id;
		});
		if (iter == game_user_session_map_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	void AddGameUserSession(uuid session_id, Ptr<GameUser> user)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		game_user_session_map_.insert(std::make_pair(session_id, user));
	}

	void RemoveGameUserSession(const uuid& session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		game_user_session_map_.erase(session_id);
	}

private:
	Ptr<IoServicePool> ios_pool_;
	Ptr<NetServer> net_server_;
	Ptr<MySQLPool> db_;

	std::mutex mutex_;
	SessionOpenedHandler session_opened_handler_;
	SessionClosedHandler session_closed_handler_;
	std::map<Game::Protocol::MessageT, MessageHandler> message_handler_map_;

	// ���� ����
	std::map<uuid,Ptr<GameUser>> game_user_session_map_;
};