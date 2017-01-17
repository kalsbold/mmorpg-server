#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <boost/asio.hpp>

#include "ByteBuffer.h"

namespace gisunnet
{
	// TCP 클라이언트와의 연결점 클래스
	class TcpTransport : public std::enable_shared_from_this<TcpTransport>
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using error_code = boost::system::error_code;

		static const std::size_t MIN_PREPARE_READ_BYTES = 1024;
		static const std::size_t MAX_READ_BUF_CAPACITY = std::numeric_limits<size_t>::max();

		TcpTransport(const TcpTransport&) = delete;
		TcpTransport& operator=(const TcpTransport&) = delete;

		// constructor
		TcpTransport(std::unique_ptr<tcp::socket> socket);
		~TcpTransport();

		void Start();

		void Close();

		void Send(BufferPtr buffer);

		bool IsOpen() const { return socket_->is_open(); }
		bool IsClosed() const { return !socket_->is_open(); }

		tcp::endpoint GetRemoteEndpoint() const { return socket_->remote_endpoint(); }

		boost::asio::io_service& GetIoService() { return socket_->get_io_service(); }

		// Callback
		// Read I/O 가 완료될때 콜백
		// buffer : 완료된 Read I/O 의 바이트 데이터가 담긴 버퍼. 처리된 바이트 만큼 ReaderIndex를 전진 시켜주어야 한다.
		// next_min_read_size : 다음 Read I/O 에 요청될 최소 바이트 사이즈.
		std::function<void(Buffer& buffer, size_t& min_next_read_bytes)> ReceiveDataHandler;
		std::function<void(void)> CloseHandler;
		std::function<void(const error_code& error)> ErrorHandler;

	private:
		bool PrepareRead(size_t min_read_bytes);
		void DoRead(size_t min_read_bytes);
		void HandleRead(const error_code& error, std::size_t bytes_transferred);
		void _Write(std::shared_ptr<Buffer>& buf);
		void DoWrite();
		void HandleWrite(const error_code& error);
		void HandleError(const error_code& error);

		std::unique_ptr<tcp::socket> socket_;
		std::unique_ptr<Buffer> read_buf_;
		std::vector<BufferPtr> pending_list_;
		std::vector<BufferPtr> sending_list_;
	};
}