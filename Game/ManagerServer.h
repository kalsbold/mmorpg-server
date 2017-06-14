#pragma once

#include <map>
#include <mutex>
#include <type_traits>
#include <chrono>
#include "Common.h"
#include "ServerBase.h"
#include "RemoteClient.h"
#include "ManagerClient.h"
#include "MySQL.h"
#include "Protocol_ss_generated.h"
#include <boost\multi_index_container.hpp>
#include <boost\multi_index\hashed_index.hpp>
#include <boost\multi_index\member.hpp>

namespace PSS = ProtocolSS;

// Manager ������ ������ Ŭ���̾�Ʈ
class RemoteManagerClient : public RemoteClient
{
public:
	using RemoteClient::RemoteClient;

	// Inherited via RemoteClient
	virtual void OnDisconnected() override {}

	std::string server_name;
	ServerType server_type;
};

// ���� ���� ����
struct UserSession
{
	UserSession(int account_uid, const uuid& credential)
		: account_uid_(account_uid), credential_(credential) {}

	int account_uid_;
	uuid credential_;

	bool login_ = true;
	time_point logout_time_;
};

using namespace boost::multi_index;

// �±׼���
struct tags
{
	struct account_uid {};
	struct credential {};
};
// �ε��� Ÿ���� ����
using indices = indexed_by<
	hashed_unique<tag<tags::account_uid>, member<UserSession, int, &UserSession::account_uid_>>,
	hashed_unique<tag<tags::credential>, member<UserSession, uuid, &UserSession::credential_>, boost::hash<boost::uuids::uuid>>
>;
// �����̳� Ÿ�� ����
using UserSessionSet = boost::multi_index_container<UserSession, indices>;

/*
�ٸ� �������� �߾� ���� ����.
�������� ����Ű �߱�, ���� �� ����.
*/
class ManagerServer : public IServer
{
public:
	ManagerServer();
	virtual ~ManagerServer();

	std::string GetName() override { return name_; }
	void SetName(const std::string& name) override { name_ = name; }

	// ���� ����
	void Run() override;
	// ���� ����
	void Stop() override;
	// ������ ����� ������ ���
	void Wait();

	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }
	const Ptr<MySQLPool>& GetDB() { return db_conn_; }
	
	const Ptr<RemoteManagerClient> GetRemoteClient(int session_id);
	const Ptr<RemoteManagerClient> GetRemoteClientByName(const std::string& name);

	void NotifyUnauthedAccess(const Ptr<net::Session>& session);
private:
	// Network message handler type.
	using MessageHandler = std::function<void(const Ptr<net::Session>&, const PSS::MessageRoot* message_root)>;

	template <typename T, typename Handler>
	void RegisterMessageHandler(Handler&& handler)
	{
		auto key = PSS::MessageTypeTraits<T>::enum_value;
		auto func = [handler = std::forward<Handler>(handler)](const Ptr<net::Session>& session, const PSS::MessageRoot* message_root)
		{
			auto* message = message_root->message_as<T>();
			handler(session, message);
		};

		message_handlers_.insert(std::pair<decltype(key), decltype(func)>(key, func));
	}

	// ������ ������Ʈ
	void DoUpdate(double delta_time);

	void AddRemoteClient(int session_id, Ptr<RemoteManagerClient> remote_client);
	void RemoveRemoteClient(int session_id);
	void RemoveRemoteClient(const Ptr<RemoteManagerClient>& remote_client);

	void ProcessRemoteClientDisconnected(const Ptr<RemoteManagerClient>& rc);

	void ScheduleNextUpdate(const time_point& now, const duration& timestep);

	// Handlers===================================================================================================
	void RegisterHandlers();

	void HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes);
	void HandleSessionOpened(const Ptr<net::Session>& session);
	void HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason);

	void OnLogin(const Ptr<net::Session>& session, const PSS::Manager::Request_Login* message);
	void OnGenerateCredential(const Ptr<net::Session>& session, const PSS::Manager::Request_GenerateCredential* message);
	void OnVerifyCredential(const Ptr<net::Session>& session, const PSS::Manager::Request_VerifyCredential* message);
	void OnUserLogout(const Ptr<net::Session>& session, const PSS::Manager::Notify_UserLogout* message);

	std::mutex mutex_;
	
	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetServer> net_server_;

	Ptr<boost::asio::strand> strand_;
	Ptr<timer> update_timer_;

	Ptr<MySQLPool> db_conn_;

	std::string name_;

	std::map<PSS::MessageType, MessageHandler> message_handlers_;
	std::map<int, Ptr<RemoteManagerClient>> remote_clients_;

	UserSessionSet user_session_set_;
};
