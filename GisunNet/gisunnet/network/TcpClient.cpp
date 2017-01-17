#include "TcpClient.h"
#include "IoServicePool.h"
#include "TcpTransport.h"

namespace gisunnet
{

TcpClient::TcpClient(IoServicePoolPtr ios_pool)
	: ios_pool_(ios_pool)
{
}

TcpClient::~TcpClient()
{
	Close();
}

void TcpClient::Connect(const std::string& host, const std::string& port)
{
	CheckClosed();

	tcp::resolver resolver(ios_pool_->PickIoService());
	auto endpoint_iterator = resolver.resolve({ host, port });
	DoConnect(endpoint_iterator);
}

void TcpClient::DoConnect(tcp::resolver::iterator endpoint_iterator)
{
	boost::asio::io_service& ios = ios_pool_->PickIoService();
	socket_ = std::make_unique<tcp::socket>(ios);
	tcp::resolver::iterator end;
	boost::asio::async_connect(*socket_, endpoint_iterator, end,
		[this, &ios](const error_code& error, tcp::resolver::iterator i)
		{
			std::lock_guard<std::mutex> guard(mutex_);
			if (closed_)
				return;

			if (error)
			{
				ConnectHandler(error, TransportPtr(nullptr));
				return;
			}

			auto transport = std::make_shared<TcpTransport>(std::move(socket_));
			transports_.push_back(transport);
				
			ConnectHandler(error, transport);
			transport->Start();;
		});
}

void TcpClient::Close()
{
	std::lock_guard<std::mutex> guard(mutex_);
	if (closed_)
		return;

	for (auto& transport : transports_)
	{
		transport->Close();
	}
	closed_ = true;
}

} // namespace gisunnet