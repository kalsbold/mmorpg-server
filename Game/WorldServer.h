#pragma once
#pragma once
#include <map>
#include <mutex>
#include <type_traits>
#include <chrono>
#include "Common.h"
#include "IServer.h"
#include "MySQL.h"
#include "protocol_cs_generated.h"

namespace PCS = ProtocolCS;

class ManagerClient;
class RemoteWorldClient;

// 게임 월드 서버.
class WorldServer : public IServer
{
public:
	WorldServer();
	virtual ~WorldServer();

	std::string GetName() override { return name_; }
	void SetName(const std::string& name) override { name_ = name; }

	// 서버 시작
	void Run() override;
	// 서버 종료
	void Stop() override;
	// 서버가 종료될 때가지 대기
	void Wait();

	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }
	const Ptr<MySQLPool>& GetDB() { return db_conn_; }

	const Ptr<RemoteWorldClient> GetRemoteClient(int session_id);
	const Ptr<RemoteWorldClient> GetRemoteClientByAccountUID(int account_uid);
	const Ptr<RemoteWorldClient> GetAuthedRemoteClient(int session_id);

	void NotifyUnauthedAccess(const Ptr<net::Session>& session);
private:
	// Network message handler type.
	using MessageHandler = std::function<void(const Ptr<net::Session>&, const PCS::MessageRoot* message_root)>;

	template <typename T, typename Handler>
	void RegisterMessageHandler(Handler&& handler)
	{
		auto key = PCS::MessageTypeTraits<T>::enum_value;
		auto func = [handler = std::forward<Handler>(handler)](const Ptr<net::Session>& session, const PCS::MessageRoot* message_root)
		{
			auto* message = message_root->message_as<T>();
			handler(session, message);
		};

		message_handlers_.insert(std::pair<decltype(key), decltype(func)>(key, func));
	}

	// 프레임 업데이트
	void DoUpdate(double delta_time) {}

	void AddRemoteClient(int session_id, Ptr<RemoteWorldClient> remote_client);
	void RemoveRemoteClient(int session_id);
	void RemoveRemoteClient(const Ptr<RemoteWorldClient>& remote_client);

	void ProcessRemoteClientDisconnected(const Ptr<RemoteWorldClient>& rc);

	void ScheduleNextUpdate(const time_point& now, const duration& timestep);

	// Handlers===================================================================================================
	void RegisterHandlers();

	void HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes);
	void HandleSessionOpened(const Ptr<net::Session>& session);
	void HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason);

	//void OnLogin(const Ptr<net::Session>& session, const PCS::World::Request_Login* message);

	// ManagerClient Handlers=======================================================================================
	void RegisterManagerClientHandlers();

	std::mutex mutex_;

	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetServer> net_server_;
	Ptr<MySQLPool> db_conn_;
	Ptr<ManagerClient> manager_client_;

	Ptr<boost::asio::strand> strand_;
	Ptr<timer> update_timer_;

	std::string name_;
	std::map<PCS::MessageType, MessageHandler> message_handlers_;
	std::map<int, Ptr<RemoteWorldClient>> remote_clients_;
};
