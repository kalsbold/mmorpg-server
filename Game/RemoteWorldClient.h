#pragma once

#include "Common.h"
#include "RemoteClient.h"
#include "DBSchema.h"

namespace db = db_schema;

class WorldServer;
class PlayerCharacter;

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

	State GetState() const
	{
		return state_;
	}

	void SetState(State state)
	{
		state_ = state;
	}

	bool IsDispose()
	{
		return disposed_;
	}

	const Ptr<db::Account>& GetAccount() const
	{
		return db_account_;
	}

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

	const Ptr<PlayerCharacter>& GetCharacter()
	{
		return character_;
	}

	void SetCharacter(Ptr<PlayerCharacter> character)
	{
		character_ = character;
	}

	void UpdateToDB();

	// 종료 처리. 상태 DB Update 등 을 한다.
	void Dispose();

	// 클라이언트에서 연결을 끊었을때 callback
	virtual void OnDisconnected() override
	{
		SetState(State::Disconnected);
		Dispose();
	}

public:
	int selected_character_uid_;

private:
	const Ptr<MySQLPool>& GetDB();

	std::atomic<bool>       disposed_;

	WorldServer*            owner_;
	Ptr<db::Account>        db_account_;
	uuid					credential_;
	
	std::atomic<State>		state_;
	Ptr<PlayerCharacter>    character_ = nullptr;
};