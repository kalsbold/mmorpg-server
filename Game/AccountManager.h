#pragma once
#include <functional>
#include "Common.h"

/*
// 인증 관리
template <typename UserT>
class AccountManager
{
public:
	using LogoutCallback = std::function<void(int account_uid, const uuid& credential)>;

	AccountManager() {};
	~AccountManager() {};

	bool CheckAndSetLogin(int account_uid, const uuid& credential, const UserT& user)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto result = acc_id_to_uuid_.insert(std::make_pair(account_uid, user));
		if (result.second)
		{
			uuid_to_acc_id_.insert(std::make_pair(credential, account_uid));
		}

		return result.second;
	}

	bool SetLogout(int account_uid)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = acc_id_to_uuid_.find(account_uid);
		if (iter == acc_id_to_uuid_.end())
			return false;
			
		logout_callback_(iter->first, iter->second);
			
		uuid_to_acc_id_.erase(iter->second);
		acc_id_to_uuid_.erase(iter);
		return true;
	}

	bool SetLogout(const uuid& credential)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = uuid_to_acc_id_.find(credential);
		if (iter == uuid_to_acc_id_.end())
			return false;

		logout_callback_(iter->second, iter->first);

		acc_id_to_uuid_.erase(iter->second);
		uuid_to_acc_id_.erase(iter);
		return true;
	}

	std::pair<bool, UserT&> CheckLogin(int account_uid)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = acc_id_to_uuid_.find(account_uid);
		if (iter == acc_id_to_uuid_.end())
		{
			return std::make_pair(false, UUID());
		}

		return std::make_pair(true, iter->second);
	}

	std::pair<bool, UserT&> CheckLogin(const uuid& credential)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = uuid_to_acc_id_.find(credential);
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
	std::map<int, Ptr<UserT>> account_uid_to_user_;
	std::map<uuid, Ptr<UserT>> credential_to_user_;
	LogoutCallback logout_callback_;
};

*/