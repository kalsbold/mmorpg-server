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
		// TO DO : lock �� �� ��� �ұ�? future promise ��� ���.
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

	virtual void RegisterMessageHandler(uint16_t message_type, const MessageHandler& handler) override
	{
		message_handler_map_.emplace(message_type, handler);
	}

private:
	using strand = boost::asio::io_service::strand;
	using SessionMap = std::map<SessionID, Ptr<Session>>;
	using MessageHandlerMap = std::map<uint16_t, MessageHandler>;

	void Listen(tcp::endpoint endpoint);
	void AcceptStart();

	// Session Handler. Session(socket)�� �Ҵ�� io_service thread ���� ȣ��ȴ�. ����ȭ �ʿ���. 
	void HandleSessionOpen(const WeakPtr<Session>& session);
	void HandleSessionClose(const WeakPtr<Session>& session, CloseReason reason);
	
	// handler
	SessionOpenedHandler	session_opened_handler_;
	SessionClosedHandler	session_closed_handler_;
	MessageHandlerMap		message_handler_map_;

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
