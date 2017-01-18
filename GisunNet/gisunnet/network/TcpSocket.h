#pragma once

#include <gisunnet/types.h>

namespace gisunnet
{
	// TCP 클라이언트와의 연결점 클래스
	class TcpSocket : public std::enable_shared_from_this<TcpSocket>
	{
	public:
		using tcp = boost::asio::ip::tcp;

		static const std::size_t MIN_PREPARE_READ_BYTES = 1024;
		static const std::size_t MAX_READ_BUF_CAPACITY = std::numeric_limits<size_t>::max();

		TcpSocket(const TcpSocket&) = delete;
		TcpSocket& operator=(const TcpSocket&) = delete;

		// constructor
		TcpSocket(std::unique_ptr<tcp::socket> socket);
		~TcpSocket();

		void Start();

		void Close();

		void Send(Ptr<Buffer> buffer);

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
		void PendWrite(Ptr<Buffer>& buf);
		void DoWrite();
		void HandleWrite(const error_code& error);
		void HandleError(const error_code& error);

		std::unique_ptr<tcp::socket> socket_;
		Ptr<Buffer> read_buf_;
		std::vector<Ptr<Buffer>> pending_list_;
		std::vector<Ptr<Buffer>> sending_list_;
	};
}