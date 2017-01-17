#include "TcpServer.h"
#include "IoServicePool.h"
#include "TcpTransport.h"

namespace gisunnet
{

TcpServer::TcpServer(IoServicePoolPtr ios_pool)
	: TcpServer(ios_pool, ios_pool)
{
}

TcpServer::TcpServer(IoServicePoolPtr acceptor_ios_pool, IoServicePoolPtr socket_ios_pool)
	: listen_ios_pool_(acceptor_ios_pool)
	, socket_ios_pool_(socket_ios_pool)
	, acceptor_ios_(listen_ios_pool_->PickIoService())
	, acceptor_(acceptor_ios_)
{
}

TcpServer::~TcpServer()
{
	Close();
}

void TcpServer::Listen(unsigned short port, boost::asio::ip::tcp protocol)
{
	if (acceptor_.is_open())
	{
		throw std::logic_error("Listen already called");
	}

	boost::asio::ip::tcp::endpoint endpoint(protocol, port);
	DoListen(endpoint);
}

void TcpServer::Listen(const std::string& host, unsigned short port)
{
	if (acceptor_.is_open())
	{
		throw std::logic_error("Listen already called");
	}

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
	DoListen(endpoint);
}

void TcpServer::Close()
{
	if (!acceptor_.is_open())
		return;

	acceptor_.close();
}

void TcpServer::DoListen(boost::asio::ip::tcp::endpoint endpoint)
{
	assert(!acceptor_.is_open());

	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen(boost::asio::ip::tcp::acceptor::max_connections);

	DoAccept();
}

void TcpServer::DoAccept()
{
	// 소켓에 할당할 io_service 객체를 얻는다
	boost::asio::io_service& socket_ios = socket_ios_pool_->PickIoService();
	socket_ = std::make_unique<tcp::socket>(socket_ios);

	acceptor_.async_accept(*socket_,
		[this, &socket_ios](const boost::system::error_code& error) mutable
		{
			if (!acceptor_.is_open())
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
				auto transport = std::make_shared<TcpTransport>(std::move(socket_));

				if (ConnectHandler)
				{
					ConnectHandler(transport);
				}

				// 시작
				socket_ios.post([transport = std::move(transport)] { transport->Start(); });
			}
			else
			{
				std::cerr << "Accept error:" << error.message() << "\n";
			}

			// 새로운 접속을 받는다
			DoAccept();
		});
}

}