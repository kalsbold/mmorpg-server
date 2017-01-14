#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <boost/asio.hpp>

#include "ByteBuffer.h"

namespace gisunnet
{
	class TcpTransport : public std::enable_shared_from_this<TcpTransport>
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using error_code = boost::system::error_code;

		// Read I/O 가 완료될때 콜백
		// read_buf : 완료된 Read I/O 의 바이트 데이터가 담긴 버퍼. 처리된 바이트 만큼 ReaderIndex를 전진 시켜주어야 한다.
		// next_min_read_size : 다음 Read I/O 에 요청될 최소 바이트 사이즈.
		using ReadHandler = std::function<void(ByteBuf& read_buf, size_t& min_next_read_bytes)>;
		
		// 소켓이 닫힐때 콜백
		using CloseHandler = std::function<void(void)>;
		
		// 에러가 발생될때 콜백
		using ErrorHandler = std::function<void(const error_code& error_msg)>;

		static const std::size_t MIN_PREPARE_READ_BYTES = 1024;
		static const std::size_t MAX_READ_BUF_CAPACITY = std::numeric_limits<size_t>::max();

		TcpTransport(const TcpTransport&) = delete;
		TcpTransport& operator=(const TcpTransport&) = delete;

		TcpTransport(std::unique_ptr<tcp::socket> socket);
		~TcpTransport();

		void Start();

		void Close();

		void Write(std::shared_ptr<ByteBuf> buffer);

		bool IsClosed() const { return !socket_->is_open(); }

		tcp::endpoint GetRemoteEndpoint() const { return socket_->remote_endpoint(); }

		boost::asio::io_service& GetIoService() { return socket_->get_io_service(); }

		// Callback
		ReadHandler ReadCallback;
		CloseHandler CloseCallback;
		ErrorHandler ErrorCallback;

	private:
		bool PrepareRead(size_t min_read_bytes);
		void DoRead(size_t min_read_bytes);
		void HandleRead(const error_code& error, std::size_t bytes_transferred);
		void _Write(std::shared_ptr<ByteBuf>& buf);
		void DoWrite();
		void HandleWrite(const error_code& error);
		void HandleError(const error_code& error);

		std::unique_ptr<tcp::socket> socket_;
		std::unique_ptr<ByteBuf> read_buf_;
		std::vector<std::shared_ptr<ByteBuf>> pending_list_;
		std::vector<std::shared_ptr<ByteBuf>> sending_list_;
	};
}