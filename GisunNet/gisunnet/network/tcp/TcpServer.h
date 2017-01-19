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
		// 설정된 io_service_pool 이 없으면 생성.
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
		// 최대 세션이면 리턴.
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
				// 세션 생성
				auto session = std::make_shared<TcpSession>(std::move(socket_));
				// Session ID 발급.
				SessionID id = boost::uuids::random_generator()();
				// 세션 객체 생성.
				auto session = std::make_shared<TcpSession>(std::move(socket_), id);
				// 핸들러 등록
				session->openHandler = [this, session]{
					HandleSessionOpen(session);
				};
				session->closeHandler = [this, session](CloseReason reason){
					HandleSessionClose(session, reason);
				};

				// 세션 리스트에 추가.
				{
					std::lock_guard<std::mutex> guard(mutex_);
					session_list_.emplace(id, session);
				}
				// 세션 시작
				session->Start();
			}
			else
			{
				std::cerr << "Accept error:" << error.message() << "\n";
			}

			// 새로운 접속을 받는다
			AcceptStart();
		}));

		accept_op_ = true;
	}

	void DoStop()
	{
		state_ = State::Stop;
		acceptor_->close();
		// 세션을 닫는다.
		for (auto& pair : session_list_)
		{
			(pair.second)->Close();
		}

		std::lock_guard<std::mutex> guard(mutex_);
		session_list_.clear();
	}

	// Session Handler
	// Session(socket)에 할당된 io_service thread 에서 호출됨 동기화 필요함. 
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
				// 세션 리스트에서 지워준다.
				std::lock_guard<std::mutex> guard(mutex_);
				session_list_.erase(id);
			}

			// 최대 세션이 아니고 Accept가 중단중 이면 Accept 재개
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
