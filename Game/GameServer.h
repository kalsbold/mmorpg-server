#pragma once

#include <map>
#include <mutex>
#include <type_traits>
#include <chrono>
#include "Common.h"
#include "ServerBase.h"
#include "MySQL.h"
#include "protocol_generated.h"

using namespace std::chrono_literals;
// 50fps
constexpr std::chrono::nanoseconds timestep(50ms);
using double_seconds = std::chrono::duration<double>;

class RemoteClient;
class Zone;
class GameObject;
class PlayerCharacter;

// ���� ����.
class GameServer : public IServer
{
public:
	// The clock type.
	using clock = std::chrono::high_resolution_clock;
	// The duration type of the clock.
	using duration = clock::duration;
	// The time point type of the clock.
	using time_point = clock::time_point;
	// Timer type
	using timer = boost::asio::high_resolution_timer;

	GameServer(){}
	~GameServer(){}

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

	// Network Session ���� RemoteClient�� ã�´�.
	const Ptr<RemoteClient> GetRemoteClient(int session_id);
	// Account ID �� RemoteClient�� ã�´�.
	const Ptr<RemoteClient> GetRemoteClientByAccountID(int account_id);

	void NotifyUnauthedAccess(const Ptr<net::Session>& session);

private:
	void AddRemoteClient(int session_id, Ptr<RemoteClient> remote_client);
	void RemoveRemoteClient(int session_id);
	void RemoveRemoteClient(const Ptr<RemoteClient>& remote_client);
	
	/*
	template <typename T>
	void BroadCast(const Ptr<net::Session>& session, fb::FlatBufferBuilder& fbb, fb::Offset<T>& message)
	{
		auto net_message = protocol::CreateNetMessage(fbb, protocol::MessageTTraits<T>::enum_value, message.Union());
		fbb.Finish(net_message);

		session->Send(fbb.GetBufferPointer(), fbb.GetSize());
	}

	template <typename T>
	void BroadCast(const Ptr<net::Session>& session, const T& message)
	{
		flatbuffers::FlatBufferBuilder fbb;
		auto offset = T::TableType::Pack(fbb, &message);
		auto net_message = CreateNetMessage(fbb, protocol::MessageTTraits<T::TableType>::enum_value, offset.Union());
		fbb.Finish(net_message);

		session->Send(fbb.GetBufferPointer(), fbb.GetSize());
	}
	*/

	// ���� ������ ������ ������ ���´�.
	void ProcessRemoteClientDisconnected(const Ptr<RemoteClient>& rc);

	// World=====================================================================================================
	void InitWorld();
	// �ʵ� �� ����
	void CreateFieldZones();
	// �ʵ� �� ��ü�� �����´�.
	Ptr<Zone> GetFieldZone(int map_id);
	// ���� ��
	void EnterZone(Ptr<PlayerCharacter> pc, Zone* zone, Vector3 pos);
	// ������ ����
	void ExitZone(Ptr<PlayerCharacter> pc);

	void ScheduleNextUpdate();

	// ������ ������Ʈ
	void DoUpdate(double delta_time)
	{

	}

	// Handlers===================================================================================================
	void RegisterHandlers();

	void HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes);
	void HandleSessionOpened(const Ptr<net::Session>& session);
	void HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason);

	void OnJoin(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnLogin(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnCreateCharacter(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnCharacterList(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnDeleteCharacter(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	
	void OnEnterWorld(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnEnterWorldNext(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);

	void OnMove(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);
	void OnAttack(const Ptr<net::Session>& session, const protocol::NetMessage* net_message);

	std::mutex mutex_;
	std::string name_;
	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetServer> net_server_;
	Ptr<MySQLPool> db_conn_;
	// Network message handler type.
	using MessageHandler = std::function<void(const Ptr<net::Session>&, const protocol::NetMessage* net_message)>;
	std::map<protocol::MessageT, MessageHandler> message_handlers_;
	// �α��� ����
	std::map<int, Ptr<RemoteClient>> remote_clients_;
	// Game world
	Ptr<boost::asio::strand>	strand_;
	Ptr<timer>					update_timer_;
	std::vector<Ptr<Zone>>		field_zones_;
};
