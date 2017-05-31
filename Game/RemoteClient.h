#pragma once
#include "Common.h"
#include "GameServer.h"
#include "World.h"
#include "Zone.h"
#include "Character.h"
#include "DBEntity.h"
#include "MessageHelper.h"

namespace fb = flatbuffers;
namespace db = db_entity;

class RemoteClient : public std::enable_shared_from_this<RemoteClient>
{
public:
	enum class WorldState
	{
		None,			// 입장 아님
		Entering,		// 입장 중
		Entered,		// 입장 됨
		Leaving,		// 나가는 중
	};

	RemoteClient(const RemoteClient&) = delete;
	RemoteClient& operator=(const RemoteClient&) = delete;

	RemoteClient(GameServer* server, const uuid& uuid, const Ptr<net::Session>& net_session)
		: server_(server)
		, uuid_(uuid)
		, net_session_(net_session)
		, state_(WorldState::None)
		, disposed_(false)
	{
		assert(server != nullptr);
		assert(net_session != nullptr);
		assert(net_session->IsOpen());
	}
	~RemoteClient()
	{
		Dispose();
	}

	const uuid& GetUUID() const
	{
		return uuid_;
	}

	const Ptr<net::Session> GetSession() const
	{
		return net_session_;
	}

	template <typename T>
	void Send(fb::FlatBufferBuilder& fbb, fb::Offset<T>& message)
	{
		helper::Send(net_session_, fbb, message);
	}

	template <typename MessageT>
	void Send(const MessageT& message)
	{
		helper::Send(net_session_, message);
	}

	WorldState GetState() const
	{
		return state_;
	}

	bool IsDisconnected()
	{
		return !net_session_->IsOpen();
	}

	// 연결을 종료한다.
	void Disconnect()
	{
		net_session_->Close();
		Dispose();
	}

	// 클라이언트에서 연결을 끊었을때 callback
	void OnDisconnected()
	{
		Dispose();
	}

	const Ptr<db::Account>& GetAccount() const
	{
		return db_account_;
	}

	bool IsAuthentication()
	{
		return db_account_ != nullptr;
	}

	void SetAuthentication(Ptr<db::Account> db_account)
	{
		db_account_ = db_account;
	}



	// 게임 입장.
	void SetCharacter(Ptr<Character> character)
	{
		state_ = WorldState::Entered;
		character_ = character;
	}

	void EnterWorld(World& world, int character_id)
	{
		// 플레이어 상태 검사
		if (GetState() != WorldState::None)
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_STATE;
			helper::Send(GetSession(), reply);
			return;
		}

		// 캐릭터 로드
		auto db_character = db::Character::Fetch(GetDB(), character_id, GetAccount()->id);
		// 캐릭터 로드 실패
		if (!db_character)
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_CHARACTER;
			helper::Send(GetSession(), reply);
			return;
		}

		// 케릭터 인스턴스 생성
		auto character = std::make_shared<Character>(boost::uuids::random_generator()(), this, db_character);

		// 입장할 Zone 객체를 얻는다.
		Zone* field_zone = world.GetFieldZone(character->map_id_);
		if (!field_zone)
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_CANNOT_ENTER_ZONE;
			Send(reply);
			return;
		}

		SetState(WorldState::Entering);
		// Zone 입장
		field_zone->Post([this, field_zone, character]()
		{
			field_zone->EnterCharacter(character);
			SetState(WorldState::Entered);
			character_ = character;
		});

		// 입장 성공 메시지
		fb::FlatBufferBuilder fbb;
		auto local_char = character->Serialize(fbb);
		auto reply = protocol::world::CreateReplyEnterWorldSuccess(fbb, local_char);
		Send(fbb, reply);
	}

	void LeaveWorld()
	{
		if (GetState() != WorldState::Entered || !character_)
			return;

		Zone* zone = character_->GetLocationZone();
		if (!zone)
			return;

		// 나감
		SetState(WorldState::Leaving);
		zone->Post([this, zone, character = character_]()
		{
			zone->LeaveCharacter(character->GetUUID());
			character->UpdateToDB();
			character_ = nullptr;
			SetState(WorldState::None);
		});
	}

	// 종료 처리. 상태 저장 등 을 한다.
	void Dispose()
	{
		bool e = false;
		if (!disposed_.compare_exchange_strong(e, true))
			return;

		LeaveWorld();
	}

private:

	const Ptr<MySQLPool>& GetDB()
	{
		return server_->GetDB();
	}

	void SetState(WorldState state)
	{
		state_ = state;
	}
	
	GameServer*             server_;
	Ptr<net::Session>       net_session_;
	uuid                    uuid_;
	
	Ptr<db::Account>        db_account_;
	
	std::atomic<WorldState>	state_;
	Ptr<Character>          character_ = nullptr;

	std::atomic<bool>       disposed_;
};
