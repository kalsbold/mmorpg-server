#pragma once

#include <map>
#include <mutex>
#include "Common.h"
#include "ServerBase.h"
#include "MySQL.h"
#include "protocol_generated.h"

class World;
class RemoteClient;

// 게임 서버.
class GameServer : public ServerBase
{
public:
	using SessionOpenedHandler = std::function<void(const Ptr<net::Session>&)>;
	using SessionClosedHandler = std::function<void(const Ptr<net::Session>&, net::CloseReason reason)>;
	using MessageHandler = std::function<void(const Ptr<net::Session>&, const protocol::NetMessage* net_message)>;

	GameServer(){}
	~GameServer(){}

	// 서버 시작
	void Run() override;
	// 서버 종료
	void Stop() override;
	// 서버가 종료될 때가지 대기
	void Wait();

	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }
	const Ptr<MySQLPool>& GetDB() { return db_conn_; }

	// Network Session 으로 RemoteClient를 찾는다.
	const Ptr<RemoteClient> GetRemoteClient(int session_id);
	// Account ID 로 RemoteClient를 찾는다.
	const Ptr<RemoteClient> GetRemoteClientByAccountID(int account_id);

	void NotifyUnauthedAccess(const Ptr<net::Session>& session);

	Ptr<World> GetWorld()
	{
		return world_;
	}
private:
	void RegisterHandlers();

	void HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes);
	void HandleSessionOpened(const Ptr<net::Session>& session);
	void HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason);

	void OnRequestLogin(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnRequestJoin(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnRequestCreateCharacter(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnRequestCharacterList(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnRequestDeleteCharacter(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnRequestEnterWorld(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);

	void AddRemoteClient(int session_id, Ptr<RemoteClient> remote_client);
	void RemoveRemoteClient(int session_id);

	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetServer> net_server_;
	Ptr<MySQLPool> db_conn_;

	std::mutex mutex_;
	SessionOpenedHandler session_opened_handler_;
	SessionClosedHandler session_closed_handler_;
	std::map<protocol::MessageT, MessageHandler> message_handlers_;
	// 유저 관리
	std::map<int, Ptr<RemoteClient>> remote_clients_;
	// Game World
	Ptr<World> world_;
};
