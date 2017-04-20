#pragma once
#include "Singleton.h"
#include "TypeDef.h"
#include "MySQL.h"
#include "DatabaseEntity.h"

namespace mmog {

	class AccountManager : public Singleton<AccountManager>
	{
	public:
		using AccountId = int;
		using LogoutCallback = function<void(AccountId, const Ptr<Session>&)>;

		AccountManager() {};
		~AccountManager() {};

		bool CheckAndSetLogin(AccountId account_id, Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			return logged_in_accounts_.insert(std::make_pair(account_id, session)).second;
		}

		bool SetLogout(AccountId account_id)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = logged_in_accounts_.find(account_id);
			if (iter != logged_in_accounts_.end())
			{
				logout_callback_(iter->first, iter->second);
				return true;
			}
			
			return false;
		}

		AccountId FindAccount(Ptr<Session>& session)
		{
			std::lock_guard<std::mutex> lock_guard(mutex_);
			auto iter = std::find_if(logged_in_accounts_.begin(), logged_in_accounts_.end(),
				[&session](const std::pair<AccountId, Ptr<Session>>& pair)
				{
					return session == pair.second;
				});

			if (iter != logged_in_accounts_.end())
			{
				return iter->first;
			}

			return 0;
		}

		const Ptr<Session> FindSession(AccountId account_id)
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
		//Ptr<MySQLPool> db_;
		std::map<int, Ptr<Session>> logged_in_accounts_;
		LogoutCallback logout_callback_;
	};
}

