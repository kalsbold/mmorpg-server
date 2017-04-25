#pragma once
#include <gisunnet/gisunnet.h>
#include "GameServer.h"
#include "Character.h"
#include "DatabaseEntity.h"

namespace mmog {

	using namespace gisunnet;

	class GamePlayer : public std::enable_shared_from_this<GamePlayer>
	{
	public:
		enum class State
		{
			Connected,		// �����
			WorldEntered,	// �����
			Disconnected,	// ���� ����
		};

		GamePlayer(GameServer* server, Ptr<Session> net_session, const Account& account_info)
			: server_(server)
			, net_session_(net_session)
			, account_info_(account_info)
			, state_(State::Connected)
		{
			assert(server != nullptr);
			assert(net_session != nullptr);
			assert(net_session->IsOpen());
		}
		~GamePlayer() {}

		const SessionID& GetSessionID() const
		{
			return net_session_->GetID();
		}

		const Ptr<Session> GetSession() const
		{
			return net_session_;
		}

		const Account& GetAccountInfo() const
		{
			return account_info_;
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
			ProcessDisconnect();
		}

		// ������ ����
		void OnDisconnected()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			ProcessDisconnect();
		}

		// ���� ����.
		void EnterGame(int character_id)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			//// �÷��̾� ���� �˻�
			//if (state_ != GameUser::State::Connected)
			//{
			//	EnterGameFailedT response;
			//	response.error_code = ErrorCode_ENTER_GAME_INVALID_STATE;
			//	helper::Send(net_session_, response);
			//	return;
			//}

			//auto db = server_->GetDB();
			//// �ɸ��� �����͸� �ҷ��´�.
			//std::stringstream ss;
			//ss << "SELECT * FROM user_character_tb "
			//	<< "WHERE id=" << character_id << " acc_id=" << account_info_.id << " AND del_type='F'";
			//auto result_set = db->Excute(ss.str());

			//if (!result_set->next())
			//{
			//	// �ɸ��Ͱ� �������� �ʴ´�. ����
			//	EnterGameFailedT response;
			//	response.error_code = ErrorCode_ENTER_GAME_INVALID_CHARACTER;
			//	Send(net_session_, response);

			//	// ������ ���´�.
			//	Disconnect();
			//	return;
			//}

			// �ʿ��� ������ �ε�

		}

	private:
		// ���� ���� ó��. ���� ���� �� �� �Ѵ�.
		void ProcessDisconnect()
		{
			state_ = State::Disconnected;
			// ����
			UpdateToDB();
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

		GameServer* server_;
		Ptr<Session> net_session_;

		std::mutex mutex_;
		Account account_info_;
		State state_;

		//Ptr<PlayerCharacter> character_;
	};
}