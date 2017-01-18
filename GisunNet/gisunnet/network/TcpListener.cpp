#include <gisunnet/network/TcpListener.h>
#include <gisunnet/network/IoServicePool.h>
#include <gisunnet/network/TcpSocket.h>

namespace gisunnet
{

TcpListener::TcpListener(Ptr<IoServicePool> ios_pool)
	: TcpListener(ios_pool, ios_pool)
{
}

TcpListener::TcpListener(Ptr<IoServicePool> acceptor_ios_pool, Ptr<IoServicePool> socket_ios_pool)
	: listen_ios_pool_(acceptor_ios_pool)
	, socket_ios_pool_(socket_ios_pool)
	, acceptor_ios_(listen_ios_pool_->PickIoService())
	, acceptor_(acceptor_ios_)
{
}

TcpListener::~TcpListener()
{
	Close();
}

void TcpListener::Listen(unsigned short port, boost::asio::ip::tcp protocol)
{
	if (acceptor_.is_open())
	{
		throw std::logic_error("Listen already called");
	}

	boost::asio::ip::tcp::endpoint endpoint(protocol, port);
	DoListen(endpoint);
}

void TcpListener::Listen(const std::string& host, unsigned short port)
{
	if (acceptor_.is_open())
	{
		throw std::logic_error("Listen already called");
	}

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
	DoListen(endpoint);
}

void TcpListener::Close()
{
	if (!acceptor_.is_open())
		return;

	acceptor_.close();
}

void TcpListener::DoListen(boost::asio::ip::tcp::endpoint endpoint)
{
	assert(!acceptor_.is_open());

	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen(boost::asio::ip::tcp::acceptor::max_connections);

	DoAccept();
}

void TcpListener::DoAccept()
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
				auto connection = std::make_shared<TcpSocket>(std::move(socket_));

				if (ConnectHandler)
				{
					ConnectHandler(connection);
				}

				// 시작
				socket_ios.post([connection = std::move(connection)] { connection->Start(); });
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