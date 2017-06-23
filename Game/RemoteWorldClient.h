#pragma once

#include "Common.h"
#include "RemoteClient.h"
#include "TypeDef.h"
#include "DBSchema.h"
#include "protocol_cs_generated.h"

namespace PCS = ProtocolCS;
namespace db = db_schema;

class WorldServer;
class World;
class Hero;

class RemoteWorldClient : public RemoteClient
{
public:
	enum class State
	{
		Connected,		// 인증후 초기상태. 월드 입장 아님
		WorldEntering,	// 월드 입장 중
		WorldEntered,	// 월드 입장 됨
		Disconnected,	// 접속 종료
	};

	RemoteWorldClient(const Ptr<net::Session>& net_session, WorldServer* owner);
	~RemoteWorldClient();
    
    // 종료 처리. 상태 DB Update 등 을 한다.
    void Dispose();

	State GetState() const { return state_; }

	void SetState(State state)
	{
		state_ = state;
	}

	bool IsDispose() { return disposed_; }

	const Ptr<db::Account>& GetAccount() const { return db_account_; }

	void SetAccount(Ptr<db::Account> db_account)
	{
		db_account_ = db_account;
	}

	bool IsAuthenticated() const
	{
		return !credential_.is_nil();
	}

	void Authenticate(uuid credential)
	{
		credential_ = credential;
	}

	const uuid& GetCredential() const
	{
		return credential_;
	}

	const Ptr<db::Hero> GetDBHero()
	{
		return db_hero_;
	}

	void SetDBHero(const Ptr<db::Hero>& db_hero)
	{
		db_hero_ = db_hero;
	}

	const Ptr<Hero>& GetHero()
	{
		return hero_;
	}

	void SetHero(Ptr<Hero> hero)
	{
		hero_ = hero;
	}

    World* GetWorld();

	void UpdateToDB();

	// 클라이언트에서 연결을 끊었을때 callback
	virtual void OnDisconnected() override
	{
		SetState(State::Disconnected);
		Dispose();
	}

public:
	int selected_hero_uid_;
    // 월드 입장
    void EnterWorld();
    // 이동
    void ActionMove(const PCS::World::Request_ActionMove * message);
    // 자신에게 주변 정보를 알린다.
    void NotifyAppearActorsToMe();

private:
	const Ptr<MySQLPool>& GetDB();

	std::atomic<bool>       disposed_;

	WorldServer*            owner_;
	Ptr<db::Account>        db_account_;
	uuid					credential_;
	
	std::atomic<State>		state_;
	Ptr<db::Hero>			db_hero_;
	Ptr<Hero>				hero_;

	time_point last_position_update_time_;
	time_point last_attack_time;

};