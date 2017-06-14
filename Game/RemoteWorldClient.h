#pragma once
/*
#include "Common.h"
#include "GameServer.h"
#include "Zone.h"
#include "Actor.h"
#include "DBSchema.h"
#include "ProtocolHelper.h"

class RemoteWorldClient : public std::enable_shared_from_this<RemoteWorldClient>
{
public:
	enum class State
	{
		Connected,		// 인증후 초기상태. 월드 입장 아님
		WorldEntering,	// 월드 입장 중
		WorldEntered,	// 월드 입장 됨
		Disconnected,	// 접속 종료
	};

	RemoteWorldClient(GameServer* server, const Ptr<net::Session>& net_session)
		: server_(server)
		, net_session_(net_session)
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

	int GetSessionID() const
	{
		return GetSession()->GetID();
	}

	const Ptr<net::Session> GetSession() const
	{
		return net_session_;
	}

	void Send(fb::FlatBufferBuilder& fbb)
	{
		if (net_session_->IsOpen())
			SendFlatBuffer(net_session_, fbb);
	}

	template <typename T>
	void Send(const T& message)
	{
		if (net_session_->IsOpen())
			SendProtocolMessage(net_session_, message);
	}

	void Send(const Ptr<net::Buffer>& buf)
	{
		if (net_session_->IsOpen())
			net_session_->Send(buf);
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

	bool IsDisconnected()
	{
		return !net_session_->IsOpen();
	}

	// 연결을 종료한다.
	void Disconnect()
	{
		net_session_->Close();
		SetState(State::Disconnected);
		Dispose();
	}

	// 클라이언트에서 연결을 끊었을때 callback
	void OnDisconnected()
	{
		SetState(State::Disconnected);
		Dispose();
	}

	const Ptr<db::Account>& GetAccount() const
	{
		return db_account_;
	}

	void SetAccount(Ptr<db::Account> db_account)
	{
		db_account_ = db_account;
	}

	bool IsAuthentication()
	{
		return !auth_key_.is_nil();
	}

	uuid GetAuthKey()
	{
		return auth_key_;
	}

	void Authenticate(uuid auth_key)
	{
		auth_key_ = auth_key;
	}

	const Ptr<PlayerCharacter>& GetCharacter()
	{
		return character_;
	}

	void SetCharacter(Ptr<PlayerCharacter> character)
	{
		character_ = character;
	}

	// 종료 처리. 상태 DB Update 등 을 한다.
	void Dispose()
	{
		bool e = false;
		if (!disposed_.compare_exchange_strong(e, true))
			return;

		// 케릭터 상태 DB Update.
		if (character_)
		{
			character_->UpdateToDB();
		}
	}

private:

	const Ptr<MySQLPool>& GetDB()
	{
		return server_->GetDB();
	}

	GameServer*             server_;
	Ptr<net::Session>       net_session_;

	uuid					auth_key_;
	Ptr<db::Account>        db_account_;

	std::atomic<State>	state_;
	Ptr<PlayerCharacter>    character_ = nullptr;

	std::atomic<bool>       disposed_;
};
*/