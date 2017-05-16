#pragma once

#include <map>
#include <mutex>
#include <flatbuffers/flatbuffers.h>
#include "ServerBase.h"
#include "MySQL.h"
#include "game_message_generated.h"

namespace mmog {
	
	using namespace std;
	namespace fb = flatbuffers;
	namespace proto = protocol;

	class World;
	class GamePlayer;

	// 게임 서버.
	class GameServer : public ServerBase
	{
	public:
		using SessionOpenedHandler = std::function<void(const Ptr<Session>&)>;
		using SessionClosedHandler = std::function<void(const Ptr<Session>&, CloseReason reason)>;
		using MessageHandler = std::function<void(const Ptr<Session>&, const proto::NetMessage* net_message)>;

		GameServer(){}
		~GameServer(){}

		// 서버 시작
		void Run() override;
		// 서버 종료
		void Stop() override;

		const Ptr<IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }
		const Ptr<MySQLPool>& GetDB() { return db_conn_; }

		// Network Session ID 로 찾는다.
		const Ptr<GamePlayer> GetGamePlayer(const SessionID& session_id);
		
		void AddGamePlayer(SessionID session_id, Ptr<GamePlayer> player);
		void RemoveGamePlayer(const SessionID& session_id);

	private:
		void RegisterHandlers();

		void HandleMessage(const Ptr<Session>& session, const uint8_t* buf, size_t bytes);
		void HandleSessionOpened(const Ptr<Session>& session);
		void HandleSessionClosed(const Ptr<Session>& session, CloseReason reason);

		void OnLogin(const Ptr<Session>& session, const proto::NetMessage* net_message);
		void OnJoin(const Ptr<Session>& session, const proto::NetMessage* net_message);
		void OnCreateCharacter(const Ptr<Session>& session, const proto::NetMessage* net_message);
		void OnCharacterList(const Ptr<Session>& session, const proto::NetMessage* net_message);
		void OnDeleteCharacter(const Ptr<Session>& session, const proto::NetMessage* net_message);
		void OnEnterGame(const Ptr<Session>& session, const proto::NetMessage* net_message);

		Ptr<IoServiceLoop> ios_loop_;
		Ptr<NetServer> net_server_;
		Ptr<MySQLPool> db_conn_;

		mutex mutex_;
		SessionOpenedHandler session_opened_handler_;
		SessionClosedHandler session_closed_handler_;
		map<proto::MessageT, MessageHandler> message_handlers_;
		// 유저 관리
		map<SessionID, Ptr<GamePlayer>> players_;
		// Game World
		Ptr<World> world_;
	};
};

