#pragma once
#include <gisunnet/gisunnet.h>
#include "GameServer.h"

namespace mmog {

	using namespace gisunnet;

	struct AccountInfo
	{
	public:
		int id = 0;
		std::string acc_name;
	};

	class GameUser
	{
	public:
		enum class State
		{
			None = 0,
			Login,			// 로그인 후
			GamePreparing,  // 입장 하기위해 준비중(데이터 로딩등)
			GameReady,		// 입장 준비 완료
			GamePlay,	    // 입장됨
			Logout,			// 로그아웃 함
		};

		GameUser(GameServer* server, const Ptr<Session>& net_session, const AccountInfo& account_info)
			: server_(server)
			, net_session_(net_session)
			, account_info_(account_info)
			, state_(State::Login)
		{}
		~GameUser() {}

		Ptr<Session> GetSession()
		{
			return net_session_;
		}

		uuid GetSessionID()
		{
			if (!net_session_)
				return uuid();

			return net_session_->ID();
		}

		AccountInfo& GetAccountInfo()
		{
			return account_info_;
		}

		int GetAccountID()
		{
			return account_info_.id;
		}

		State GetState()
		{
			return state_;
		}

		// 연결을 종료한다.
		void Close()
		{
			net_session_->Close();
		}

		// 연결이 끊김
		void OnDisconnected()
		{

		}

		// 게임 입장 시퀀스를 시작한다.
		void BeginEnterGame(int hero_id)
		{
			auto strand = net_session_->GetStrand();
			strand.post([this, hero_id]()
			{
				state_ = State::GamePreparing;

				auto db = server_->GetDB();
				// 영웅 데이터를 불러온다.
				std::stringstream ss;
				ss << "SELECT * FROM user_hero_tb "
					<< "WHERE id=" << hero_id << " acc_id=" << account_info_.id << " AND del_type='F'";
				auto result_set = db->Excute(ss.str());

				if (!result_set->next())
				{
					// 케릭터가 존재하지 않는다. 실패
					EnterGameFailedReplyT reply;
					reply.error_code = ErrorCode_ENTER_GAME_INVALID_HERO;
					Send(net_session_, reply);
					return;
				}

			});
		}

	private:
		GameServer* server_;
		Ptr<Session> net_session_;
		std::mutex mutex_;
		AccountInfo account_info_;
		State state_ = State::None;

		Hero* hero_;
	};
}