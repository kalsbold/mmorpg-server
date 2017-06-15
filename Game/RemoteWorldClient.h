#pragma once

#include "Common.h"
#include "RemoteClient.h"
#include "GameServer.h"
#include "Zone.h"
#include "Actor.h"
#include "DBSchema.h"

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

	RemoteWorldClient(const Ptr<net::Session>& net_session, WorldServer* owner)
		: RemoteClient(net_session)
		, owner_(owner)
		, state_(State::Connected)
		, disposed_(false)
	{
		assert(server != nullptr);
		assert(net_session != nullptr);
		assert(net_session->IsOpen());
	}
	~RemoteWorldClient()
	{
		Dispose();
	}

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

	// ���� ó��. ���� DB Update �� �� �Ѵ�.
	void Dispose()
	{
		bool exp = false;
		if (!disposed_.compare_exchange_strong(exp, true))
			return;

		// �ɸ��� ���� DB Update.
		if (character_)
		{
			character_->UpdateToDB();
		}
	}

	// Ŭ���̾�Ʈ���� ������ �������� callback
	virtual void OnDisconnected() override
	{
		SetState(State::Disconnected);
		Dispose();
	}

private:

	const Ptr<MySQLPool>& GetDB()
	{
		return owner_->GetDB();
	}

	std::atomic<bool>       disposed_;

	WorldServer*            owner_;
	Ptr<db::Account>        db_account_;
	uuid					credential_;
	
	std::atomic<State>		state_;
	Ptr<PlayerCharacter>    character_ = nullptr;

	
};