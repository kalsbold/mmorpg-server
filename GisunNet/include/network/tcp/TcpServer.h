#pragma once

#include <mutex>
#include <set>
#include "include/network/NetServer.h"
#include "include/network/tcp/TcpSession.h"
#include "include/network/IoServiceLoop.h"

namespace gisun {
namespace net {

	class TcpServer : public NetServer, public std::enable_shared_from_this<TcpServer>
	{
	public:
		using tcp = boost::asio::ip::tcp;

		TcpServer(const ServerConfig& config);
		virtual ~TcpServer() override;

		virtual void Start(uint16_t port) override;
		virtual void Start(std::string address, uint16_t port) override;
		virtual void Stop() override;

		State GetState() override
		{
			return state_;
		}

		Ptr<IoServiceLoop> GetIoServiceLoop() override
		{
			return ios_loop_;
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

		void Listen(tcp::endpoint endpoint);
		void AcceptStart();

		// Session Handler. Session(socket)에 할당된 io_service thread 에서 호출된다. 동기화 필요함. 
		void HandleSessionOpen(const Ptr<TcpSession>& session);
		void HandleSessionClose(const Ptr<TcpSession>& session, CloseReason reason);
		void HandleSessionReceive(const Ptr<TcpSession>& session, const uint8_t* buf, size_t bytes);

		// handler
		SessionOpenedHandler	session_opened_handler_;
		SessionClosedHandler	session_closed_handler_;
		MessageHandler			message_handler_;

		ServerConfig			config_;
		State					state_;
		Ptr<IoServiceLoop>		ios_loop_;
		std::mutex				mutex_;
		std::set<Ptr<TcpSession>>	sessions_;

		std::unique_ptr<strand> strand_;
		std::unique_ptr<tcp::acceptor> acceptor_;
		// bool accept_op_ = false;
		// The next socket to be accepted.
		std::unique_ptr<tcp::socket> socket_;
	};

} // namespace net
} // namespace gisun
