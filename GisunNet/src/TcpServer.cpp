#include "network/tcp/TcpServer.h"

namespace gisun {
namespace net {

TcpServer::TcpServer(const ServerConfig & config)
	: NetServer()
	, config_(config)
	, state_(State::Ready)
{
	// ������ io_service_loop �� ������ ����.
	if (config_.io_service_loop == nullptr)
	{
		size_t thread_count = std::max<size_t>(config_.thread_count, 1);
		ios_loop_ = std::make_shared<IoServiceLoop>(thread_count);
	}
	else
	{
		ios_loop_ = config_.io_service_loop;
	}
	
	strand_ = std::make_unique<strand>(ios_loop_->GetIoService());

	// Free session id list �� �����. 
	for (size_t i = 0; i < config_.max_session_count; i++)
	{
		free_session_id_.push_back(i);
	}
}

TcpServer::~TcpServer()
{
	Stop();
}

void TcpServer::Start(uint16_t port)
{
	Start("0.0.0.0", port);
}

void TcpServer::Start(std::string address, uint16_t port)
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);

	strand_->dispatch([this, self = shared_from_this(), endpoint]
	{
		if (!(state_ == State::Ready))
		{
			BOOST_LOG_TRIVIAL(info) << "TcpServer can't start : State is not Ready\n";
			return;
		}

		Listen(endpoint);
		AcceptStart();
		state_ = State::Start;

		BOOST_LOG_TRIVIAL(info) << "TcpServer start " << endpoint;
	});
}

void TcpServer::Stop()
{
	strand_->dispatch([this, self = shared_from_this()]
	{
		if (!(state_ == State::Start))
		return;

		acceptor_->close();
		// ��� ������ �ݴ´�.
		for (auto& pair : sessions_)
		{
			pair.second->Close();
		}
		sessions_.clear();
		free_session_id_.clear();

		state_ = State::Stop;

		BOOST_LOG_TRIVIAL(info) << "TcpServer stop";
	});
}

void TcpServer::Listen(tcp::endpoint endpoint)
{
	auto acceptor = std::make_unique<tcp::acceptor>(strand_->get_io_service());

	acceptor->open(endpoint.protocol());
	acceptor->set_option(tcp::acceptor::reuse_address(true));
	acceptor->bind(endpoint);
	acceptor->listen(tcp::acceptor::max_connections);

	acceptor_ = std::move(acceptor);
}

inline void TcpServer::AcceptStart()
{
	// Create tcp socket
	socket_ = std::make_unique<tcp::socket>(ios_loop_->GetIoService());
	// Async accept
	acceptor_->async_accept(*socket_, strand_->wrap([this, self = shared_from_this()](error_code error) mutable
	{
		if (state_ == State::Stop)
		{
			error_code ec;
			socket_->shutdown(tcp::socket::shutdown_both, ec);
			socket_->close();
			socket_.reset();
			return;
		}

		// �ִ� �����̸� �ݾ���.
		if (sessions_.size() >= config_.max_session_count)
		{
			error_code ec;
			socket_->shutdown(tcp::socket::shutdown_both, ec);
			socket_->close();
			socket_.reset();
			//accept_op_ = false;
			//return;
			BOOST_LOG_TRIVIAL(info) << "Session is full. max_session_count:" << config_.max_session_count;
		}
		else if (!error)
		{
			// id �߱�
			int id = free_session_id_.front();
			free_session_id_.pop_front();
			// ���� ��ü ����.
			auto session = std::make_shared<TcpSession>(std::move(socket_), id, config_);
			// �ڵ鷯 ���
			session->open_handler = [this](auto& s) {
				HandleSessionOpen(s);
			};
			session->close_handler = [this](auto& s, CloseReason reason) {
				HandleSessionClose(s, reason);
			};
			session->recv_handler = [this](auto& s, const uint8_t* buf, size_t bytes) {
				HandleSessionReceive(s, buf, bytes);
			};

			// ���� ����Ʈ�� �߰�.
			{
				std::lock_guard<std::mutex> guard(mutex_);
				sessions_.emplace(session->GetID(), session);
			}
			// ���� ����
			session->Start();
		}
		else
		{
			BOOST_LOG_TRIVIAL(info) << "Accept error : " << error.message();
		}

		// ���ο� ������ �޴´�
		AcceptStart();
	}));

	//accept_op_ = true;
}

inline void TcpServer::HandleSessionOpen(const Ptr<TcpSession>& session)
{
	if (session_opened_handler_)
		session_opened_handler_(session);
}

inline void TcpServer::HandleSessionClose(const Ptr<TcpSession>& session, CloseReason reason)
{
	if (session_closed_handler_)
		session_closed_handler_(session, reason);

	strand_->dispatch([this, id = session->GetID()]
	{	
		// ���� ����Ʈ���� �����ش�.
		{
			std::lock_guard<std::mutex> guard(mutex_);
			sessions_.erase(id);
		}
		// id �� ������
		free_session_id_.push_back(id);
	});
}

inline void TcpServer::HandleSessionReceive(const Ptr<TcpSession>& session, const uint8_t* buf, size_t bytes)
{
	if (message_handler_)
		message_handler_(session, buf, bytes);
}

} // namespace net
} // namespace gisun
