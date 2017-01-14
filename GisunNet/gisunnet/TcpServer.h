#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <boost/asio.hpp>

namespace gisunnet
{
	class IoServicePool;
	class TcpTransport;

	// Tcp ������ �޾� ���̴� Ŭ����
	class TcpServer
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using IoServicePoolPtr = std::shared_ptr<IoServicePool>;
		using TransportPtr = std::shared_ptr<TcpTransport>;

		// ���� ���ӵɶ� �ݹ�
		using AcceptedHandler = std::function<void(TransportPtr&)>;

		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		explicit TcpServer(IoServicePoolPtr ios_pool);
		~TcpServer();

		// Start the server
		void Start(unsigned short port, tcp protocol = tcp::v4());
		void Start(const std::string& host, const std::string& service);

		// Stop the server
		void Stop();

		// Wait the server
		void Wait();

		// Callback
		AcceptedHandler AcceptedCallback;

	private:
		void Listen(boost::asio::ip::tcp::endpoint endpoint);
		void DoAccept();

		// io_service pool.
		IoServicePoolPtr ios_pool_;
		
		// acceptor_ios �� �����ϴ� ������.
		std::thread acceptor_thread_;
		boost::asio::io_service acceptor_ios_;
		tcp::acceptor acceptor_;
		
		// The next socket to be accepted.
		std::unique_ptr<tcp::socket> socket_;
	};
}