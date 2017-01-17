#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <boost/asio.hpp>

#include "ByteBuffer.h"

namespace gisunnet
{
	// TCP Ŭ���̾�Ʈ���� ������ Ŭ����
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
		// Read I/O �� �Ϸ�ɶ� �ݹ�
		// buffer : �Ϸ�� Read I/O �� ����Ʈ �����Ͱ� ��� ����. ó���� ����Ʈ ��ŭ ReaderIndex�� ���� �����־�� �Ѵ�.
		// next_min_read_size : ���� Read I/O �� ��û�� �ּ� ����Ʈ ������.
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