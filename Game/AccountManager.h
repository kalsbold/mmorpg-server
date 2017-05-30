#pragma once
#include <functional>
#include "Common.h"

// 인증 관리
class AccountManager
{
public:
	using AccountID = int;
	using UUID = uuid;
	using LogoutCallback = std::function<void(AccountID, UUID)>;

	AccountManager() {};
	~AccountManager() {};

	bool CheckAndSetLogin(AccountID account_id, UUID uuid)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto result = acc_id_to_uuid_.insert(std::make_pair(account_id, uuid));
		if (result.second)
		{
			uuid_to_acc_id_.insert(std::make_pair(uuid, account_id));
		}

		return result.second;
	}

	bool SetLogout(AccountID account_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = acc_id_to_uuid_.find(account_id);
		if (iter == acc_id_to_uuid_.end())
			return false;
			
		logout_callback_(iter->first, iter->second);
			
		uuid_to_acc_id_.erase(iter->second);
		acc_id_to_uuid_.erase(iter);
		return true;
	}

	bool SetLogout(UUID uuid)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = uuid_to_acc_id_.find(uuid);
		if (iter == uuid_to_acc_id_.end())
			return false;

		logout_callback_(iter->second, iter->first);

		acc_id_to_uuid_.erase(iter->second);
		uuid_to_acc_id_.erase(iter);
		return true;
	}

	std::pair<bool, UUID> CheckLogin(AccountID account_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = acc_id_to_uuid_.find(account_id);
		if (iter == acc_id_to_uuid_.end())
		{
			return std::make_pair(false, UUID());
		}

		return std::make_pair(true, iter->second);
	}

	std::pair<bool, AccountID> CheckLogin(UUID uuid)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = uuid_to_acc_id_.find(uuid);
		if (iter == uuid_to_acc_id_.end())
		{
			return std::make_pair(false, 0);
		}

		return std::make_pair(true, iter->second);
	}

	void RegisterLogoutHandler(const LogoutCallback& handler)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		logout_callback_ = handler;
	}

private:
	std::mutex mutex_;
	std::map<AccountID, UUID> acc_id_to_uuid_;
	std::map<UUID, AccountID> uuid_to_acc_id_;
	LogoutCallback logout_callback_;
};

