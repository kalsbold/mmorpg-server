#pragma once
#include <gisunnet/gisunnet.h>

using namespace gisunnet;

struct AccountInfo
{
public:
	int id = 0;
	std::string acc_name;
};

class GameUserSession
{
public:
	GameUserSession(WeakPtr<Session> net_session, const AccountInfo& account_info)
		: net_session_(net_session), account_info_(account_info)
	{}
	~GameUserSession() {}

	AccountInfo& GetAccountInfo()
	{
		return account_info_;
	}

private:
	WeakPtr<Session> net_session_;
	AccountInfo account_info_;
	
};