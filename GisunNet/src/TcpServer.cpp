#include "gisunnet/network/tcp/TcpServer.h"

namespace gisunnet {

TcpServer::TcpServer(const Configuration & config)
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

TcpServer::~TcpServer()
{
	Stop();
}

void TcpServer::Start(uint16_t port)
{
	Start("0.0.0.0", port);
}

void TcpServer::Start(string address, uint16_t port)
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
	strand_->dispatch([this, endpoint]()
	{
		if (!(state_ == State::Ready))
			return;
		
		Listen(endpoint);
		AcceptStart();
		state_ = State::Start;
		
		std::cerr << "Server start :" << endpoint << "\n";
	});
}

void TcpServer::Stop()
{
	// TO DO : future ������� ���� ��� ��ȯ�ޱ�?
	strand_->dispatch([this]()
	{
		acceptor_->close();
		// ��� ������ �ݴ´�.
		for (auto& pair : session_list_)
		{
			(pair.second)->Close();
		}
		
		// TO DO : lock ����� �� ���ϼ� ������?
		{
			std::lock_guard<std::mutex> guard(mutex_);
			session_list_.clear();
		}
		state_ = State::Stop;

		std::cerr << "Server stop\n";
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
	socket_ = std::make_unique<tcp::socket>(ios_pool_->PickIoService());
	// Async accept
	acceptor_->async_accept(*socket_, strand_->wrap([this](error_code error) mutable
	{
		if (state_ == State::Stop)
		{
			//if (!error)
			//{
				socket_->shutdown(tcp::socket::shutdown_both);
				socket_->close();
			//}
			socket_.release();
			return;
		}

		// �ִ� �����̸� �ݾ���.
		if (session_list_.size() >= config_.max_session_count)
		{
			socket_->shutdown(tcp::socket::shutdown_both);
			socket_->close();
			socket_.release();
			//accept_op_ = false;
			//return;
		}
		else if (!error)
		{
			// Session ID �߱�.
			SessionID id = boost::uuids::random_generator()();
			// ���� ��ü ����.
			auto session = std::make_shared<TcpSession>(std::move(socket_), id, config_);
			// �ڵ鷯 ���
			// TO DO : ��ü ������ �ѱ�°� ���⽺����.. weak_ptr�� �ƴϰ� shared_ptr�� �ѱ�� �޸� ������ �ȵȴ�! signal2 �� ������ ����غ���(track).
			WeakPtr<TcpSession> session_weak = session;
			session->openHandler = [this, session_weak] {
				HandleSessionOpen(session_weak);
			};
			session->closeHandler = [this, session_weak](CloseReason reason) {
				HandleSessionClose(session_weak, reason);
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

	//accept_op_ = true;
}

inline void TcpServer::HandleSessionOpen(const WeakPtr<Session>& session)
{
	auto s = session.lock();
	if (!s) return;
	if (session_opened_handler_)
		session_opened_handler_(s);
}

inline void TcpServer::HandleSessionClose(const WeakPtr<Session>& session, CloseReason reason)
{
	auto s = session.lock();
	if (!s) return;
	if (session_closed_handler_)
		session_closed_handler_(s, reason);

	strand_->dispatch([this, id = s->ID()]
	{
		{
			// ���� ����Ʈ���� �����ش�.
			std::lock_guard<std::mutex> guard(mutex_);
			session_list_.erase(id);
		}

		// Accept �簳 ���� �˻�
		//if (session_list_.size() < config_.max_session_count && !accept_op_)
		//{
			// Accept �簳
			//AcceptStart();
		//}
	});
}

} // namespace gisunnet
