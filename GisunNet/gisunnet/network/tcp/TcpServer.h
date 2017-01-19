#pragma once

#include <gisunnet/network/Server.h>
#include <gisunnet/network/Configuration.h>
#include <gisunnet/network/tcp/TcpSession.h>
#include <gisunnet/network/IoServicePool.h>

namespace gisunnet {

class TcpServer : public Server
{
public:
	using tcp = boost::asio::ip::tcp;

	TcpServer(const Configuration& config)
		: Server()
		, config_(config)
		, state_(State::Ready)
	{
		// ������ io_service_pool �� ������ ����.
		if (config_.io_service_pool.get() == nullptr)
		{
			size_t thread_count = std::max<size_t>(config_.thread_count, 1);
			config_.io_service_pool = std::make_shared<IoServicePool>(thread_count);
		}
		ios_pool_ = config_.io_service_pool;
		strand_ = std::make_unique<strand>(ios_pool_->PickIoService());
	}

	virtual ~TcpServer() override
	{
		Stop();
	}

	virtual void Start(uint16_t port) override
	{
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port);
		strand_->dispatch([this, endpoint] { DoStart(endpoint);  });
	}

	virtual void Start(string address, uint16_t port) override
	{
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
		strand_->dispatch([this, endpoint] { DoStart(endpoint);  });
	}

	virtual void Stop() override
	{
		strand_->dispatch([this] { DoStop(); });
	}

	State GetState() override
	{
		return state_;
	}

	Ptr<Session> Find(const SessionID& id) override
	{
		std::lock_guard<std::mutex> guard(mutex_);
		auto it = session_list_.find(id);
		return (it != session_list_.end()) ? (*it).second : Session::NullPtr;
	}

	// Register Handler
	virtual void RegisterSessionOpenedHandler(const SessionOpenedHandler& handler) override
	{
		sessionOpenedHandler_ = handler;
	}

	virtual void RegisterSessionClosedHandler(const SessionClosedHandler& handler) override
	{
		sessionClosedHandler_ = handler;
	}

	virtual void RegisterMessageHandler(uint16_t message_type, const MessageHandler& handler) override
	{
		messageHandlerMap_.emplace(message_type, handler);
	}

private:
	void DoStart(tcp::endpoint endpoint)
	{
		if (!(state_ == State::Ready))
			return;

		state_ = State::Start;
		Listen(endpoint);
	}

	void Listen(tcp::endpoint endpoint)
	{
		acceptor_ = std::make_unique<tcp::acceptor>(strand_->get_io_service());

		acceptor_->open(endpoint.protocol());
		acceptor_->set_option(tcp::acceptor::reuse_address(true));
		acceptor_->bind(endpoint);
		acceptor_->listen(tcp::acceptor::max_connections);

		AcceptStart();
	}

	void AcceptStart()
	{
		// �ִ� �����̸� ����.
		if (session_list_.size() >= config_.max_session_count)
		{
			accept_op_ = false;
			return;
		}
		
		// Create tcp socket
		socket_ = std::make_unique<tcp::socket>(ios_pool_->PickIoService());
		// Async accept
		acceptor_->async_accept(*socket_, strand_->wrap([this](error_code error) mutable
		{
			if (state_ == State::Stop)
			{
				if (!error)
				{
					socket_->shutdown(tcp::socket::shutdown_both);
					socket_->close();
				}
				socket_.release();
				return;
			}

			if (!error)
			{
				// ���� ����
				auto session = std::make_shared<TcpSession>(std::move(socket_));
				// Session ID �߱�.
				SessionID id = boost::uuids::random_generator()();
				// ���� ��ü ����.
				auto session = std::make_shared<TcpSession>(std::move(socket_), id);
				// �ڵ鷯 ���
				session->openHandler = [this, session]{
					HandleSessionOpen(session);
				};
				session->closeHandler = [this, session](CloseReason reason){
					HandleSessionClose(session, reason);
				};

				// ���� ����Ʈ�� �߰�.
				{
					std::lock_guard<std::mutex> guard(mutex_);
					session_list_.emplace(id, session);
				}
				// ���� ����
				session->Start();
			}
			else
			{
				std::cerr << "Accept error:" << error.message() << "\n";
			}

			// ���ο� ������ �޴´�
			AcceptStart();
		}));

		accept_op_ = true;
	}

	void DoStop()
	{
		state_ = State::Stop;
		acceptor_->close();
		// ������ �ݴ´�.
		for (auto& pair : session_list_)
		{
			(pair.second)->Close();
		}

		std::lock_guard<std::mutex> guard(mutex_);
		session_list_.clear();
	}

	// Session Handler
	// Session(socket)�� �Ҵ�� io_service thread ���� ȣ��� ����ȭ �ʿ���. 
	void HandleSessionOpen(const Ptr<Session>& session)
	{
		if (sessionOpenedHandler_)
			sessionOpenedHandler_(session);
	}

	void HandleSessionClose(const Ptr<Session>& session, CloseReason reason)
	{
		if (sessionClosedHandler_)
			sessionClosedHandler_(session, reason);

		strand_->dispatch([this, id = session->ID()]
		{
			{
				// ���� ����Ʈ���� �����ش�.
				std::lock_guard<std::mutex> guard(mutex_);
				session_list_.erase(id);
			}

			// �ִ� ������ �ƴϰ� Accept�� �ߴ��� �̸� Accept �簳
			if (session_list_.size() < config_.max_session_count && !accept_op_)
			{
				AcceptStart();
			}
		});
	}

	using strand = boost::asio::io_service::strand;
	using SessionMap = std::map<SessionID, Ptr<Session>>;
	using MessageHandlerMap = std::map<uint16_t, MessageHandler>;
	
	// handler
	SessionOpenedHandler	sessionOpenedHandler_;
	SessionClosedHandler	sessionClosedHandler_;
	MessageHandlerMap		messageHandlerMap_;

	Configuration	config_;
	State			state_;
	Ptr<IoServicePool> ios_pool_;
	std::mutex		mutex_;
	SessionMap		session_list_;

	std::unique_ptr<strand> strand_;
	std::unique_ptr<tcp::acceptor> acceptor_;
	bool accept_op_ = false;
	// The next socket to be accepted.
	std::unique_ptr<tcp::socket> socket_;
};

}
