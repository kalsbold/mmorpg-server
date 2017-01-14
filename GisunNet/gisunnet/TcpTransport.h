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

		// Read I/O �� �Ϸ�ɶ� �ݹ�
		// read_buf : �Ϸ�� Read I/O �� ����Ʈ �����Ͱ� ��� ����. ó���� ����Ʈ ��ŭ ReaderIndex�� ���� �����־�� �Ѵ�.
		// next_min_read_size : ���� Read I/O �� ��û�� �ּ� ����Ʈ ������.
		using ReadHandler = std::function<void(ByteBuf& read_buf, size_t& min_next_read_bytes)>;
		
		// ������ ������ �ݹ�
		using CloseHandler = std::function<void(void)>;
		
		// ������ �߻��ɶ� �ݹ�
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