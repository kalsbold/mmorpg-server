#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <gisunnet/types.h>

namespace gisunnet
{
	class IoServicePool;
	class TcpSocket;

	// Tcp 접속을 받아 들이는 클래스
	class TcpListener
	{
	public:
		using tcp = boost::asio::ip::tcp;

		TcpListener(const TcpListener&) = delete;
		TcpListener& operator=(const TcpListener&) = delete;

		explicit TcpListener(Ptr<IoServicePool> ios_pool);
		TcpListener(Ptr<IoServicePool> listen_ios_pool, Ptr<IoServicePool> socket_ios_pool);
		~TcpListener();

		// Listen
		void Listen(unsigned short port, tcp protocol = tcp::v4());
		void Listen(const std::string& host, unsigned short port);

		// Close the server
		void Close();

		// Callback
		// TCP 클라이언트와 연결이 수락 되었을때 호출됨.
		std::function<void(Ptr<TcpSocket>&)> ConnectHandler;

	private:
		void DoListen(boost::asio::ip::tcp::endpoint endpoint);
		void DoAccept();

		Ptr<IoServicePool> listen_ios_pool_;
		Ptr<IoServicePool> socket_ios_pool_;
		// acceptor
		boost::asio::io_service& acceptor_ios_;
		tcp::acceptor acceptor_;
		// The next socket to be accepted.
		std::unique_ptr<tcp::socket> socket_;
	};
}