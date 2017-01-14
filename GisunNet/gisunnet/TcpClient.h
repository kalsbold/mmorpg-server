#pragma once

#include <boost/asio.hpp>

namespace gisunnet
{
	class IoServicePool;
	class TcpTransport;

	class TcpClient
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using TransportPtr = std::shared_ptr<TcpTransport>;

		TcpClient();
		~TcpClient();

	private:

	};

	TcpClient::TcpClient()
	{
	}

	TcpClient::~TcpClient()
	{
	}
}