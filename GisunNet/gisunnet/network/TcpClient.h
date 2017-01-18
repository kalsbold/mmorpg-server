#pragma once

#include <mutex>
#include <boost/asio.hpp>

namespace gisunnet
{
	class IoServicePool;
	class TcpSocket;

	class TcpClient
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using error_code = boost::system::error_code;
		using IoServicePoolPtr = std::shared_ptr<IoServicePool>;
		using TransportPtr = std::shared_ptr<TcpSocket>;

		TcpClient(IoServicePoolPtr ios_pool);
		~TcpClient();

		void Connect(const std::string& host, const std::string& port);
		
		void Close();

		std::function<void(const error_code&, TransportPtr&)> ConnectHandler;

	private:
		void DoConnect(tcp::resolver::iterator endpoint_iterator);
		void CheckClosed()
		{
			std::lock_guard<std::mutex> guard(mutex_);
			if (closed_)
			{
				throw std::logic_error("Client is closed");
			}
		}

		IoServicePoolPtr ios_pool_;
		std::unique_ptr<tcp::socket> socket_ = nullptr;
		std::list<TransportPtr> transports_;
		bool closed_ = false;
		std::mutex mutex_;
	};

	
}