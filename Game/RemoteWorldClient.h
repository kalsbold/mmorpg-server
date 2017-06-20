#pragma once

#include "Common.h"
#include "RemoteClient.h"
#include "TypeDef.h"
#include "DBSchema.h"

namespace db = db_schema;

class WorldServer;
class Hero;

class RemoteWorldClient : public RemoteClient
{
public:
	enum class State
	{
		Connected,		// ������ �ʱ����. ���� ���� �ƴ�
		WorldEntering,	// ���� ���� ��
		WorldEntered,	// ���� ���� ��
		Disconnected,	// ���� ����
	};

	RemoteWorldClient(const Ptr<net::Session>& net_session, WorldServer* owner);
	~RemoteWorldClient();

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

	void UpdateToDB();

	// ���� ó��. ���� DB Update �� �� �Ѵ�.
	void Dispose();

	// Ŭ���̾�Ʈ���� ������ �������� callback
	virtual void OnDisconnected() override
	{
		SetState(State::Disconnected);
		Dispose();
	}

public:
	int selected_hero_uid_;

	// �޽��� ó��
    void OnActionMove(const Vector3& position, float rotation, const Vector3& velocity);

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