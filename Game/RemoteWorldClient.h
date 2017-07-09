#pragma once

#include "Common.h"
#include "RemoteClient.h"
#include "TypeDef.h"
#include "DBSchema.h"
#include "protocol_cs_generated.h"
#include "ClientInterestArea.h"

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
	virtual ~RemoteWorldClient();
    
    // 종료 처리. 상태 DB Update 등 을 한다.
    void Dispose();
    bool IsDispose() const { return disposed_; }

    // 상태
	State GetState() const { return state_; }
	void SetState(State state) { state_ = state; }

    // 계정 정보
	const Ptr<db::Account>& GetAccount() const { return db_account_; }
	void SetAccount(Ptr<db::Account> db_account) { db_account_ = db_account; }

    // 인증서 저장
    void Authenticate(uuid credential) { credential_ = credential; }
    // 인증 확인
	bool IsAuthenticated() const { return !credential_.is_nil(); }

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

    const Ptr<MySQLPool>& GetDB();
	
    void UpdateToDB();

	// 클라이언트에서 연결을 끊었을때 callback
	virtual void OnDisconnected() override
	{
        RemoteClient::OnDisconnected();

		SetState(State::Disconnected);
		Dispose();
	}
	
    // 월드 입장
    void EnterWorld();
    // 이동
    void ActionMove(const PCS::World::Request_ActionMove * message);
    // 스킬 사용
    void ActionSkill(const PCS::World::Request_ActionSkill * message);
    // 부활
    void Respawn();

    int selected_hero_uid_;

private:
    void ExitWorld();

    // callback handler
    void OnUpdateHeroPosition(const Vector3& position)
    {
        if (interest_area_)
        {
            interest_area_->Position(position);
            interest_area_->UpdateInterest();
        }
    }

    void OnHeroDeath();

	std::atomic<bool>       disposed_;

	WorldServer*            owner_;
	Ptr<db::Account>        db_account_;
	uuid					credential_;
	
	std::atomic<State>		state_;
	Ptr<db::Hero>			db_hero_;
	Ptr<Hero>				hero_;
    Ptr<ClientInterestArea> interest_area_;

	time_point last_position_update_time_;
	time_point last_attack_time_;

    Ptr<timer_type> respawn_timer_;
};