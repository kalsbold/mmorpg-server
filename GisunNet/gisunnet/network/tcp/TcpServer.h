#pragma once

#include <gisunnet/network/Server.h>
#include <gisunnet/network/Configuration.h>
#include <gisunnet/network/tcp/TcpSession.h>
#include <gisunnet/network/IoServicePool.h>

namespace gisunnet {

class TcpServer : public Server
{
public:
	using SessionID = uuid;
	using tcp = boost::asio::ip::tcp;

	TcpServer(const Configuration& config)
		: config_(config)
		, state_(State::Uninit)
	{
		Initialize();
	}

	virtual ~TcpServer() override
	{
		Stop();
	}

	virtual void Start(uint16_t port) override
	{
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port);
		DoStart(endpoint);
	}

	virtual void Start(string address, uint16_t port) override
	{
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
		DoStart(endpoint);
	}

	virtual void Stop() override
	{
		std::lock_guard<std::mutex> lock(mutex_);
		server_->Close();
		state_ = State::Stop;
		// 技记 甘阑 厚款促.
		sessions_.clear();
	}

	State GetState()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return state_;
	}

private:
	using SessionMap = std::map<SessionID, std::unique_ptr<Session>>;

	void Initialize()
	{
		// 汲沥等 io_service_pool 捞 绝栏搁 积己.
		if (config_.io_service_pool.get() == nullptr)
		{
			size_t thread_count = std::max<size_t>(config_.thread_count, 1);
			config_.io_service_pool = std::make_shared<IoServicePool>(thread_count);
		}
		ios_pool_ = config_.io_service_pool;
		acceptor_ios_ = &(ios_pool_->PickIoService());

		state_ = State::Ready;
	}

	void DoStart(tcp::endpoint endpoint)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		
		if (!(state_ == State::Ready))
		{
			throw std::logic_error("Server is not State::Ready");
		}

		state_ = State::Start;
		DoListen(endpoint);
	}

	void DoListen(tcp::endpoint endpoint)
	{
		acceptor_ = std::make_unique<tcp::acceptor>(*acceptor_ios_);

		acceptor_->open(endpoint.protocol());
		acceptor_->set_option(tcp::acceptor::reuse_address(true));
		acceptor_->bind(endpoint);
		acceptor_->listen(tcp::acceptor::max_connections);

		DoAccept();
	}

	void DoAccept()
	{
		// Create tcp socket
		socket_ = std::make_unique<tcp::socket>(ios_pool_->PickIoService());

		acceptor_->async_accept(*socket_, [this](error_code error) mutable
		{
			if (!acceptor_->is_open())
			{
				if (!error)
				{
					socket_->shutdown(tcp::socket::shutdown_both);
					socket_->close();
				}
				return;
			}

			if (!error)
			{
				// 技记 积己
				auto session = std::make_shared<TcpSession>(std::move(socket_));
				// Session ID 惯鞭.
				SessionID id = boost::uuids::random_generator()();
				// 技记 按眉 积己.
				auto session = std::make_shared<TcpSession>(std::move(socket_), id);
				session->openHandler = [this, session]
				{
					if (sessionOpenedHandler)
						sessionOpenedHandler(session);
				};

				session->closeHandler = [this, session](CloseReason reason)
				{
					if (sessionClosedHandler)
						sessionClosedHandler(session, reason);

					// 技记 府胶飘俊辑 瘤款促.
					std::lock_guard<std::mutex> lock(mutex_);
					session_list_.erase(session->ID());
				};

				std::lock_guard<std::mutex> lock(mutex_);
				// 技记 府胶飘俊 眠啊.
				session_list_.emplace(id, session);
				// 技记 矫累
				session->Start();
			}
			else
			{
				std::cerr << "Accept error:" << error.message() << "\n";
			}

			// 货肺款 立加阑 罐绰促
			DoAccept();
		});
	}

	// Session Handler
	void HandleSessionOpen(Ptr<Session>& session)
	{
		if (sessionOpenedHandler)
			sessionOpenedHandler(session);
	}

	void HandleSessionStop(Ptr<Session>& session, CloseReason reason)
	{
		if (sessionClosedHandler)
			sessionClosedHandler(session, reason);

		std::lock_guard<std::mutex> lock(mutex_);
		session_list_.erase(session->ID());
	}

	std::mutex		mutex_;
	Configuration	config_;
	State			state_;
	Ptr<IoServicePool> ios_pool_;
	SessionMap		session_list_;

	// acceptor
	boost::asio::io_service* acceptor_ios_;
	std::unique_ptr<tcp::acceptor> acceptor_;
	// The next socket to be accepted.
	std::unique_ptr<tcp::socket> socket_;
};

}
