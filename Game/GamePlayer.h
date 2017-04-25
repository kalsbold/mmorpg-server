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
			Connected,		// 연결됨
			WorldEntered,	// 입장됨
			Disconnected,	// 접속 종료
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

		// 연결을 종료한다.
		void Disconnect()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (net_session_ == nullptr)
				return;

			net_session_->Close();
			ProcessDisconnect();
		}

		// 연결이 끊김
		void OnDisconnected()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			ProcessDisconnect();
		}

		// 게임 입장.
		void EnterGame(int character_id)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			//// 플레이어 상태 검사
			//if (state_ != GameUser::State::Connected)
			//{
			//	EnterGameFailedT response;
			//	response.error_code = ErrorCode_ENTER_GAME_INVALID_STATE;
			//	helper::Send(net_session_, response);
			//	return;
			//}

			//auto db = server_->GetDB();
			//// 케릭터 데이터를 불러온다.
			//std::stringstream ss;
			//ss << "SELECT * FROM user_character_tb "
			//	<< "WHERE id=" << character_id << " acc_id=" << account_info_.id << " AND del_type='F'";
			//auto result_set = db->Excute(ss.str());

			//if (!result_set->next())
			//{
			//	// 케릭터가 존재하지 않는다. 실패
			//	EnterGameFailedT response;
			//	response.error_code = ErrorCode_ENTER_GAME_INVALID_CHARACTER;
			//	Send(net_session_, response);

			//	// 연결을 끊는다.
			//	Disconnect();
			//	return;
			//}

			// 필요한 데이터 로딩

		}

	private:
		// 세션 종료 처리. 상태 저장 등 을 한다.
		void ProcessDisconnect()
		{
			state_ = State::Disconnected;
			// 저장
			UpdateToDB();
		}

		// 데이터를 DB에 저장.
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