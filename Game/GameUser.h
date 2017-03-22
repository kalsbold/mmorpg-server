#pragma once
#include <gisunnet/gisunnet.h>

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
	enum State
	{
		None = 0,				// ���� ���� ���� ����(�α��� ��)
		EnterWorldPreparing,	// �����ϱ����� �غ���(������ �ε���)
		EnterWorldReady,		// ���� �غ� �Ϸ�
		EnteredWorld,			// �����
	};

	GameUser(WeakPtr<Session> net_session, const AccountInfo& account_info)
		: net_session_(net_session), account_info_(account_info)
	{}
	~GameUser() {}

	AccountInfo& GetAccountInfo()
	{
		return account_info_;
	}

	State GetState()
	{
		return state_;
	}

private:
	WeakPtr<Session> net_session_;
	AccountInfo account_info_;
	State state_ = State::None;
	
};