#include "TcpServer.h"
#include "IoServicePool.h"
#include "TcpTransport.h"

namespace gisunnet
{

TcpServer::TcpServer(IoServicePoolPtr ios_pool)
	: ios_pool_(ios_pool)
	, acceptor_(acceptor_ios_)
{
}

TcpServer::~TcpServer()
{
	Stop();
}

void TcpServer::Start(unsigned short port, boost::asio::ip::tcp protocol)
{
	if (acceptor_.is_open())
		return;

	boost::asio::ip::tcp::endpoint endpoint(protocol, port);
	Listen(endpoint);
}

void TcpServer::Start(const std::string& host, const std::string& service)
{
	if (acceptor_.is_open())
		return;

	boost::asio::ip::tcp::resolver resolver(acceptor_ios_);
	boost::asio::ip::tcp::resolver::query query(host, service);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	Listen(endpoint);
}

void TcpServer::Stop()
{
	acceptor_ios_.stop();
}

void TcpServer::Wait()
{
	if (acceptor_thread_.joinable())
		acceptor_thread_.join();
}

void TcpServer::Listen(boost::asio::ip::tcp::endpoint endpoint)
{
	assert(!acceptor_.is_open());

	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen(boost::asio::ip::tcp::acceptor::max_connections);

	DoAccept();

	acceptor_thread_ = std::thread([this]
	{
		acceptor_ios_.run();
	});
}

void TcpServer::DoAccept()
{
	// 소켓에 할당할 io_service 객체를 얻는다
	boost::asio::io_service& socket_ios = ios_pool_->PickIoService();
	socket_ = std::move(std::make_unique<tcp::socket>(socket_ios));

	acceptor_.async_accept(*socket_,
		[this, &socket_ios](const boost::system::error_code& error) mutable
	{
		if (!error)
		{
			auto transport = std::make_shared<TcpTransport>(std::move(socket_));

			if (AcceptedCallback)
			{
				AcceptedCallback(transport);
			}

			// 시작
			socket_ios.post([transport] { transport->Start(); });
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