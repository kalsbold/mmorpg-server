#pragma once
#include <gisunnet/gisunnet.h>

using namespace gisunnet;

struct AccountInfo
{
public:
	int id = 0;
	std::string acc_name;
};

class GameSession
{
public:
	GameSession(WeakPtr<Session> net_session, const AccountInfo& acc)
		: net_session_(net_session), acc_(acc)
	{}
	~GameSession() {}

	AccountInfo& GetAccountInfo()
	{
		return acc_;
	}

private:
	WeakPtr<Session> net_session_;
	AccountInfo acc_;
	
};