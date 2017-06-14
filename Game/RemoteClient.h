#pragma once
#include "Common.h"

class RemoteClient : public std::enable_shared_from_this<RemoteClient>
{
public:
	RemoteClient(const RemoteClient&) = delete;
	RemoteClient& operator=(const RemoteClient&) = delete;

	RemoteClient(const Ptr<net::Session>& net_session)
		: net_session_(net_session)
	{
		assert(net_session != nullptr);
		assert(net_session->IsOpen());
	}

	virtual ~RemoteClient()
	{
	}

	int GetSessionID() const
	{
		return net_session_->GetID();
	}

	const Ptr<net::Session>& GetSession() const
	{
		return net_session_;
	}

	void Send(const uint8_t * data, size_t size)
	{
		net_session_->Send(data, size);
	}

	template <typename BufferT>
	void Send(BufferT&& data)
	{
		net_session_->Send(std::forward<BufferT>(data));
	}

	bool IsDisconnected()
	{
		return !net_session_->IsOpen();
	}

	// 연결을 종료한다.
	void Disconnect()
	{
		net_session_->Close();
	}

	// 연결이 끊겼을때 callback
	virtual void OnDisconnected() = 0;

private:
	Ptr<net::Session> net_session_;
};
