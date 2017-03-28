#pragma once

#include <map>
#include <mutex>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>
#include "MySQL.h"
#include "TypeDef.h"
#include "game_message_generated.h"

namespace mmog {

	using namespace gisunnet;

	class GameUser;

	// 게임 서버.
	class GameServer
	{
	public:
		using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
		using SessionClosedHandler = std::function<void(const Ptr<Session>&, CloseReason reason)>;
		using MessageHandler = std::function<void(const Ptr<Session>&, const protocol::NetMessage* net_message)>;

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

		void RegisterSessionOpenHandler(const SessionOpenedHandler& handler)
		{
			session_opened_handler_ = handler;
		}
		void RegisterSessionCloseHandler(const SessionClosedHandler& handler)
		{
			session_closed_handler_ = handler;
		}
		void RegisterMessageHandler(const protocol::MessageT& message_type, const MessageHandler& message_handler)
		{
			message_handler_map_.insert(std::make_pair(message_type, message_handler));
		}

		// Message Handler =======================================================
		// Join
		void OnJoinRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Login
		void OnLoginRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Create Hero
		void OnCreateHeroRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Hero List
		void OnHeroListRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Delete Hero
		void OnDeleteHeroRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Enter Game
		void OnEnterGameRequest(const Ptr<Session>& session, const protocol::NetMessage* net_message);

		const Ptr<GameUser> FindGameUser(const uuid& session_id);
		// Account ID로 찾는다
		const Ptr<GameUser> FindGameUser(int account_id);
		void AddGameUser(uuid session_id, Ptr<GameUser> user);
		void RemoveGameUser(const uuid& session_id);

	private:
		void InitializeHandlers();
		// Session Open & Close Handler ===============================================================
		void OnSessionOpen(const Ptr<Session>& session);
		void OnSessionClose(const Ptr<Session>& session, CloseReason reason);

		Ptr<IoServicePool> ios_pool_;
		Ptr<NetServer> net_server_;
		Ptr<MySQLPool> db_;

		std::mutex mutex_;
		SessionOpenedHandler session_opened_handler_;
		SessionClosedHandler session_closed_handler_;
		std::map<protocol::MessageT, MessageHandler> message_handler_map_;

		// 유저 관리
		std::map<uuid, Ptr<GameUser>> user_map_;
	};
};

