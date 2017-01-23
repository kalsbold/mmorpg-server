#pragma once

#include <mutex>
#include "gisunnet/network/Server.h"
#include <gisunnet/network/tcp/TcpSession.h>
#include "gisunnet/network/IoServicePool.h"

namespace gisunnet {

class TcpServer : public Server
{
public:
	using tcp = boost::asio::ip::tcp;

	TcpServer(const Configuration& config);
	virtual ~TcpServer() override;

	virtual void Start(uint16_t port) override;
	virtual void Start(string address, uint16_t port) override;
	virtual void Stop() override;

	State GetState() override
	{
		return state_;
	}

	Ptr<Session> Find(const SessionID& id) override
	{
		// TO DO : lock 을 꼭 써야 할까? future promise 사용 고려.
		std::lock_guard<std::mutex> guard(mutex_);
		auto it = session_list_.find(id);
		return (it != session_list_.end()) ? (*it).second : Session::NullPtr;
	}

	// Register Handler
	virtual void RegisterSessionOpenedHandler(const SessionOpenedHandler& handler) override
	{
		session_opened_handler_ = handler;
	}

	virtual void RegisterSessionClosedHandler(const SessionClosedHandler& handler) override
	{
		session_closed_handler_ = handler;
	}

	virtual void RegisterMessageHandler(const MessageHandler& handler) override
	{
		message_handler_ = handler;
	}

private:
	using strand = boost::asio::io_service::strand;
	using SessionMap = std::map<SessionID, Ptr<Session>>;

	void Listen(tcp::endpoint endpoint);
	void AcceptStart();

	// Session Handler. Session(socket)에 할당된 io_service thread 에서 호출된다. 동기화 필요함. 
	void HandleSessionOpen(const WeakPtr<Session>& session);
	void HandleSessionClose(const WeakPtr<Session>& session, CloseReason reason);
	void HandleSessionReceive(const WeakPtr<Session>& session, const uint8_t* buf, size_t bytes);
	
	// handler
	SessionOpenedHandler	session_opened_handler_;
	SessionClosedHandler	session_closed_handler_;
	MessageHandler		message_handler_;

	Configuration	config_;
	State			state_;
	Ptr<IoServicePool> ios_pool_;
	std::mutex		mutex_;
	SessionMap		session_list_;

	std::unique_ptr<strand> strand_;
	std::unique_ptr<tcp::acceptor> acceptor_;
	// bool accept_op_ = false;
	// The next socket to be accepted.
	std::unique_ptr<tcp::socket> socket_;
};

} // namespace gisunnet
