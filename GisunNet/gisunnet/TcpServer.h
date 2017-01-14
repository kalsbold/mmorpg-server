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

	// Tcp 접속을 받아 들이는 클래스
	class TcpServer
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using IoServicePoolPtr = std::shared_ptr<IoServicePool>;
		using TransportPtr = std::shared_ptr<TcpTransport>;

		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		explicit TcpServer(IoServicePoolPtr ios_pool);
		TcpServer(IoServicePoolPtr listen_ios_pool, IoServicePoolPtr socket_ios_pool);
		~TcpServer();

		// Listen the server
		void Listen(unsigned short port, tcp protocol = tcp::v4());
		void Listen(const std::string& host, unsigned short port);

		// Close the server
		void Close();

		// Callback
		// TCP 클라이언트와 연결이 수락 되었을때 호출됨.
		std::function<void(TransportPtr&)> ConnectHandler;

	private:
		void DoListen(boost::asio::ip::tcp::endpoint endpoint);
		void DoAccept();

		IoServicePoolPtr listen_ios_pool_;
		IoServicePoolPtr socket_ios_pool_;
		// acceptor
		boost::asio::io_service& acceptor_ios_;
		tcp::acceptor acceptor_;
		// The next socket to be accepted.
		std::unique_ptr<tcp::socket> socket_ = nullptr;
	};
}