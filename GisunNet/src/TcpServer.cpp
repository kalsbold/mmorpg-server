#include "gisunnet/network/tcp/TcpServer.h"

namespace gisunnet {

TcpServer::TcpServer(const Configuration & config)
	: NetServer()
	, config_(config)
	, state_(State::Ready)
{
	// 설정된 io_service_loop 이 없으면 생성.
	if (config_.io_service_loop.get() == nullptr)
	{
		size_t thread_count = std::max<size_t>(config_.thread_count, 1);
		config_.io_service_loop = std::make_shared<IoServiceLoop>(thread_count);
	}
	ios_loop_ = config_.io_service_loop;
	strand_ = std::make_unique<strand>(ios_loop_->GetIoService());
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
	
	std::lock_guard<std::mutex> guard(mutex_);
	if (!(state_ == State::Ready))
	{
		BOOST_LOG_TRIVIAL(info) << "TcpServer can't start : State is not Ready\n";
		return;
	}

	Listen(endpoint);
	AcceptStart();
	state_ = State::Start;
		
	BOOST_LOG_TRIVIAL(info) << "TcpServer start " << endpoint;
}

void TcpServer::Stop()
{
	std::lock_guard<std::mutex> guard(mutex_);
	if (!(state_ == State::Start))
	{
		return;
	}

	acceptor_->close();
	// 모든 세션을 닫는다.
	for (auto& pair : session_list_)
	{
		(pair.second)->Close();
	}
	
	session_list_.clear();
	state_ = State::Stop;

	BOOST_LOG_TRIVIAL(info) << "TcpServer stop";
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
	acceptor_->async_accept(*socket_, strand_->wrap([this, self = shared_from_this() ](error_code error) mutable
	{
		if (state_ == State::Stop)
		{
			//if (!error)
			//{
				error_code ec;
				socket_->shutdown(tcp::socket::shutdown_both, ec);
				socket_->close();
			//}
			socket_.reset();
			return;
		}

		// 최대 세션이면 닫아줌.
		if (session_list_.size() >= config_.max_session_count)
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
			// Session ID 발급.
			auto id = boost::uuids::random_generator()();
			// 세션 객체 생성.
			auto session = std::make_shared<TcpSession>(std::move(socket_), id, config_);
			// 핸들러 등록
			// TO DO : 객체 소유권 넘기는게 번잡스럽다.. weak_ptr이 아니고 shared_ptr로 넘기면 메모리 해제가 안된다! signal2 로 변경을 고려해본다(track 기능).
			WeakPtr<TcpSession> session_weak = session;
			session->openHandler = [this, session_weak] {
				HandleSessionOpen(session_weak);
			};
			session->closeHandler = [this, session_weak](CloseReason reason) {
				HandleSessionClose(session_weak, reason);
			};
			session->recvHandler = [this, session_weak](const uint8_t* buf, size_t bytes) {
				HandleSessionReceive(session_weak, buf, bytes);
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
			BOOST_LOG_TRIVIAL(info) << "Accept error : " << error.message();
		}

		// 새로운 접속을 받는다
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

	// 세션 리스트에서 지워준다.
	std::lock_guard<std::mutex> guard(mutex_);
	session_list_.erase(s->GetID());
}

inline void TcpServer::HandleSessionReceive(const WeakPtr<Session>& session, const uint8_t* buf, size_t bytes)
{
	auto s = session.lock();
	if (!s) return;
	if (message_handler_)
		message_handler_(s, buf, bytes);
}

} // namespace gisunnet
