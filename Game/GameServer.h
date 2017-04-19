#pragma once

#include <map>
#include <mutex>
#include <flatbuffers/flatbuffers.h>
#include "ServerBase.h"
#include "MySQL.h"
#include "game_message_generated.h"

namespace mmog {

	class GameUser;

	// ���� ����.
	class GameServer : public ServerBase
	{
	public:
		using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
		using SessionClosedHandler = std::function<void(const Ptr<Session>&, CloseReason reason)>;
		using MessageHandler = std::function<void(const Ptr<Session>&, const protocol::NetMessage* net_message)>;

		GameServer(){}
		~GameServer(){}

		void Run() override;

		void Stop() override;

		const Ptr<IoServicePool>& GetIoServicePool() { return ios_pool_; }

		const Ptr<MySQLPool>& GetDB() { return db_; }

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

		// Network Session ID �� ã�´�.
		const Ptr<GameUser> GetGameUser(const SessionID& session_id);
		// User UUID�� ã�´�.
		const Ptr<GameUser> GetGameUserByUserID(const uuid& user_id);
		// Account ID�� ã�´�.
		const Ptr<GameUser> GetGameUserByAccountID(int account_id);
		// GameUser �߰�
		void AddGameUser(SessionID session_id, Ptr<GameUser> user);
		// GameUser ����
		void RemoveGameUser(const SessionID& session_id);

	private:
		void RegisterHandlers();
		// Session Open & Close Handler ===============================================================
		void OnSessionOpen(const Ptr<Session>& session);
		void OnSessionClose(const Ptr<Session>& session, CloseReason reason);

		// Message Handler =======================================================
		// Join
		void OnRequestJoin(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Login
		void OnRequestLogin(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Create Character
		void OnRequestCreateCharacter(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Character List
		void OnRequestCharacterList(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Delete Character
		void OnRequestDeleteCharacter(const Ptr<Session>& session, const protocol::NetMessage* net_message);
		// Enter Game
		void OnRequestEnterGame(const Ptr<Session>& session, const protocol::NetMessage* net_message);

		Ptr<IoServicePool> ios_pool_;
		Ptr<NetServer> net_server_;
		Ptr<MySQLPool> db_;

		std::mutex mutex_;
		SessionOpenedHandler session_opened_handler_;
		SessionClosedHandler session_closed_handler_;
		std::map<protocol::MessageT, MessageHandler> message_handler_map_;

		// ���� ����
		std::map<SessionID, Ptr<GameUser>> user_map_;
	};
};

