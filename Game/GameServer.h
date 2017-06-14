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

// 게임 서버.
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

	// 서버 측에서 강제로 접속을 끊는다.
	void ProcessRemoteClientDisconnected(const Ptr<RemoteClient>& rc);

	// World=====================================================================================================
	void InitWorld();
	// 필드 존 생성
	void CreateFieldZones();
	// 필드 존 객체를 가져온다.
	Ptr<Zone> GetFieldZone(int map_id);
	// 존에 들어감
	void EnterZone(Ptr<PlayerCharacter> pc, Zone* zone, Vector3 pos);
	// 존에서 나감
	void ExitZone(Ptr<PlayerCharacter> pc);

	void ScheduleNextUpdate();

	// 프레임 업데이트
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
	// 로그인 유저
	std::map<int, Ptr<RemoteClient>> remote_clients_;
	// Game world
	Ptr<boost::asio::strand>	strand_;
	Ptr<timer>					update_timer_;
	std::vector<Ptr<Zone>>		field_zones_;
};
