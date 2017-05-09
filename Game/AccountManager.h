#pragma once
#include "Singleton.h"
#include "TypeDef.h"

namespace mmog {

	// 로그인 유저 관리
	class AccountManager : public Singleton<AccountManager>
	{
	public:
		using AccountID = int;
		using LogoutCallback = function<void(int, const Ptr<Session>&)>;

		AccountManager() {};
		~AccountManager() {};

		bool CheckAndSetLogin(AccountID account_id, const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			bool result = id_to_session_.insert(std::make_pair(account_id, session)).second;
			if (result)
			{
				session_to_id_.insert(std::make_pair(session, account_id));
			}

			return result;
		}

		bool SetLogout(AccountID account_id)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = id_to_session_.find(account_id);
			if (iter == id_to_session_.end())
				return false;
			
			logout_callback_(iter->first, iter->second);
			
			session_to_id_.erase(iter->second);
			id_to_session_.erase(iter);
			return true;
		}

		bool SetLogout(const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = session_to_id_.find(session);
			if (iter == session_to_id_.end())
				return false;

			logout_callback_(iter->second, iter->first);

			id_to_session_.erase(iter->second);
			session_to_id_.erase(iter);
			return true;
		}

		AccountID FindAccount(const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = session_to_id_.find(session);
			if (iter == session_to_id_.end())
				return 0;

			return iter->second;
		}

		const Ptr<Session> FindSession(int account_id)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = id_to_session_.find(account_id);
			if (iter == id_to_session_.end())
				return nullptr;

			return iter->second;
		}

		void RegisterLogoutHandler(const LogoutCallback& handler)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			logout_callback_ = handler;
		}

	private:
		std::mutex mutex_;
		std::map<AccountID, Ptr<Session>> id_to_session_;
		std::map<Ptr<Session>, AccountID> session_to_id_;
		LogoutCallback logout_callback_;
	};
}

