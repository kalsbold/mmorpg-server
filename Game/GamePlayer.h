#pragma once
#include <gisunnet/gisunnet.h>
#include "GameServer.h"
#include "Character.h"
#include "DBEntity.h"
#include "MessageHelper.h"

namespace mmog {

	using namespace std;
	namespace fb = flatbuffers;
	namespace proto = protocol;
	namespace db = db_entity;

	class GamePlayer : public std::enable_shared_from_this<GamePlayer>
	{
	public:
		enum class State
		{
			Connected = 0,		// 연결됨
			WorldEntered,	// 입장됨
			Disconnected,	// 접속 종료
		};

		GamePlayer(GameServer* server, World* world, const Ptr<Session>& net_session, int account_id)
			: server_(server)
			, world_(world)
			, net_session_(net_session)
			, account_id_(account_id)
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

		int GetAccountID() const
		{
			return account_id_;
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
		bool EnterGame(int character_id)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			
			// 플레이어 상태 검사
			if (state_ != State::Connected)
			{
				proto::EnterGameFailedT response;
				response.error_code = proto::ErrorCode_ENTER_GAME_INVALID_STATE;
				helper::Send(net_session_, response);
				return false;
			}

			// 캐릭터 로드
			auto db_character = db::Character::Fetch(server_->GetDB(), character_id, GetAccountID());
			if (!db_character)
			{
				// 캐릭터가 존재하지 않는다. 실패
				proto::EnterGameFailedT response;
				response.error_code = proto::ErrorCode_ENTER_GAME_INVALID_CHARACTER;
				helper::Send(net_session_, response);
				return false;
			}



			// 필요한 데이터 로딩
			

		}

	private:
		// 세션 종료 처리. 상태 저장 등 을 한다.
		void ProcessDisconnect()
		{
			if (state_ == State::Disconnected)
				return;

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

		bool LoadCharacter(int character_id)
		{
			db_character_ = db::Character::Fetch(server_->GetDB(), character_id, account_id_);
		}

		GameServer* server_;
		World* world_;
		Ptr<Session> net_session_;

		std::mutex mutex_;
		int account_id_;
		State state_;

		Ptr<db::Character> db_character_;
		mmog::Character* go_character_;
	};
}