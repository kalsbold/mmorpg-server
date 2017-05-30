#pragma once
#include "Common.h"
#include "GameServer.h"
#include "World.h"
#include "Character.h"
#include "DBEntity.h"
#include "MessageHelper.h"

namespace fb = flatbuffers;
namespace db = db_entity;

class RemoteClient : public std::enable_shared_from_this<RemoteClient>
{
public:
	enum class State
	{
		Connect = 0,	// �����
		Login,			// �α���
		WorldEnter,		// ���� ����
		Disconnect,		// ���� ����
	};

	static Ptr<RemoteClient> Create(GameServer* server, const uuid& uuid, const Ptr<net::Session>& net_session)
	{
		return std::make_shared<RemoteClient>(server, uuid, net_session);
	}

	RemoteClient(GameServer* server, const uuid& uuid, const Ptr<net::Session>& net_session)
		: server_(server)
		, world_(server->GetWorld().get())
		, uuid_(uuid)
		, net_session_(net_session)
		, state_(State::Connect)
		, dispose_(false)
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

	template <typename MessageT>
	void Send(const MessageT& message)
	{
		helper::Send(net_session_, message);
	}

	State GetState() const
	{
		return state_;
	}

	// ������ �����Ѵ�.
	void Disconnect()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (net_session_ == nullptr)
			return;

		net_session_->Close();
		Dispose();
	}

	// Ŭ���̾�Ʈ���� ������ �������� callback
	void OnDisconnect()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		Dispose();
	}

	const Ptr<db::Account>& GetAccount() const
	{
		return db_account_;
	}

	void SetLogin(Ptr<db::Account> db_account)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		db_account_ = db_account;
		state_ = State::Login;
	}

	// ���� ����.
	void EnterWorld(int character_id)
	{
		std::lock_guard<std::mutex> lock(mutex_);
			
		// �÷��̾� ���� �˻�
		if (state_ != State::Connect)
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_STATE;
			helper::Send(net_session_, reply);
			return;
		}

		// ĳ���� �ε�
		auto db_character = db::Character::Fetch(server_->GetDB(), character_id, db_account_->id);
		// ĳ���� �ε� ����
		if (!db_character)
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_CHARACTER;
			helper::Send(net_session_, reply);
			return;
		}

		// �ɸ��� �ν��Ͻ� ����
		auto character = Character::Create();
		character->db_data_ = db_character.get();
		character->remote_client_ = this;

		if (!world_->EnterCharacter(character))
		{
			protocol::world::ReplyEnterWorldFailedT reply;
			reply.error_code = protocol::ErrorCode::ENTER_WORLD_CANNOT_ENTER_ZONE;
			helper::Send(net_session_, reply);
		}

		db_character_ = db_character;
		character_ = character;

		state_ = State::WorldEnter;

		//helper::Send(net_session_, reply);
	}

private:
	// ���� ���� ó��. ���� ���� �� �� �Ѵ�.
	void Dispose()
	{
		if (dispose_)
			return;

		// ����
		UpdateToDB();

		dispose_ = true;
	}

	// �����͸� DB�� ����.
	void UpdateToDB()
	{
		/*if (!character_)
			return;

		auto db = server_->GetDB();
		std::stringstream ss;
		ss << "UPDATE user_character_tb SET "
			<< "exp=" << character_->exp_
			<< "level=" << character_->level_
			<< "hp=" << character_->hp_
			<< "mp=" << character_->mp_
			<< "att=" << character_->att_
			<< "def=" << character_->def_
			<< "map_id=" << character_->map_id_
			<< "pos_x=" << character_->pos_.X
			<< "pos_y=" << character_->pos_.Y
			<< "pos_z=" << character_->pos_.Z
			<< "rotation_y=" << character_->rotation_y
			<< "WHERE id=" << character_->id_;
		auto result_set = db->Excute(ss.str());*/
	}

	std::mutex         mutex_;
	bool               dispose_;

	GameServer*        server_;
	Ptr<net::Session>  net_session_;
		
	uuid               uuid_;
	State              state_;
	Ptr<db::Account>   db_account_;
	Ptr<db::Character> db_character_;
		
	World*             world_ = nullptr;
	Ptr<Character>     character_ = nullptr;
};
