#pragma once
#include "Common.h"
#include "RemoteClient.h"
#include "DBSchema.h"

namespace fb = flatbuffers;
namespace db = db_schema;


class RemoteLoginClient : public RemoteClient
{
public:
	RemoteLoginClient(const RemoteLoginClient&) = delete;
	RemoteLoginClient& operator=(const RemoteLoginClient&) = delete;

	RemoteLoginClient(const Ptr<net::Session>& net_session, Ptr<db::Account> db_account)
		: RemoteClient(net_session)
		, db_account_(db_account)
		, disposed_(false)
	{
		assert(net_session);
		assert(net_session->IsOpen());
		assert(db_account);
	}

	~RemoteLoginClient()
	{
		Dispose();
	}

	bool IsDisposed() const
	{
		return disposed_;
	}

	const Ptr<db::Account>& GetAccount() const
	{
		return db_account_;
	}

	bool IsAuthenticated() const
	{
		return !credential_.is_nil();
	}

	void Authenticate(uuid credential)
	{
		credential_ = credential;
	}

	const uuid& GetCredential() const
	{
		return credential_;
	}

	// 종료 처리. 상태 DB Update 등 을 한다.
	void Dispose()
	{
		bool exp = false;
		if (!disposed_.compare_exchange_strong(exp, true))
			return;


	}

	virtual void OnDisconnected() override
	{
		Dispose();
	}

private:
	std::atomic<bool>       disposed_;
	Ptr<db::Account>		db_account_;

	uuid					credential_;
};
