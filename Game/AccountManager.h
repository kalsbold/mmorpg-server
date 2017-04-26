#pragma once
#include "Singleton.h"
#include "TypeDef.h"

namespace mmog {

	// 로그인 유저 관리
	class AccountManager : public Singleton<AccountManager>
	{
	public:
		using LogoutCallback = function<void(int, const Ptr<Session>&)>;

		AccountManager() {};
		~AccountManager() {};

		bool CheckAndSetLogin(int account_id, const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			return logged_in_accounts_.insert(std::make_pair(account_id, session)).second;
		}

		bool SetLogout(int account_id)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = logged_in_accounts_.find(account_id);
			if (iter == logged_in_accounts_.end())
				return false;
			
			logout_callback_(iter->first, iter->second);
			logged_in_accounts_.erase(iter);
			return true;
		}

		bool SetLogout(const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = std::find_if(logged_in_accounts_.begin(), logged_in_accounts_.end(),
				[&session](const std::pair<int, Ptr<Session>>& pair)
				{
					return session == pair.second;
				});
			if (iter == logged_in_accounts_.end())
				return false;

			logout_callback_(iter->first, iter->second);
			logged_in_accounts_.erase(iter);
			return true;
		}

		int FindAccount(const Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = std::find_if(logged_in_accounts_.begin(), logged_in_accounts_.end(),
				[&session](const std::pair<int, Ptr<Session>>& pair)
				{
					return session == pair.second;
				});

			if (iter == logged_in_accounts_.end())
				return 0;

			return iter->first;
		}

		const Ptr<Session> FindSession(int account_id)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = logged_in_accounts_.find(account_id);
			if (iter != logged_in_accounts_.end())
			{
				return iter->second;
			}

			return nullptr;
		}

		void RegisterLogoutHandler(const LogoutCallback& handler)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			logout_callback_ = handler;
		}

	private:
		std::mutex mutex_;
		std::map<int, Ptr<Session>> logged_in_accounts_;
		LogoutCallback logout_callback_;
	};
}

