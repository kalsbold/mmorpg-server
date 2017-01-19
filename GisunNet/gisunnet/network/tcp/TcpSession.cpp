#include <gisunnet/network/tcp/TcpSession.h>


namespace gisunnet {
	inline TcpSession::TcpSession(const SessionID & id, std::unique_ptr<tcp::socket> socket, const Configuration & config)
		: id_(id)
		, socket_(std::move(socket))
	{
		assert(socket_.get() != nullptr);
		assert(socket_->is_open());

		strand_ = std::make_unique<strand>(socket_->get_io_service());
		remote_endpoint_ = socket_->remote_endpoint();
		state_ = State::Opening;

		// Set config
		no_delay_ = config.no_delay;
		min_receive_size_ = config.min_receive_size;
		max_receive_buffer_size_ = config.max_receive_buffer_size;
		max_transfer_size_ = config.max_transfer_size;
	}
	inline TcpSession::~TcpSession()
	{

	}

	// Inherited via Session

	inline const SessionID & TcpSession::ID() const
	{
		return id_;
	}
	inline bool TcpSession::GetRemoteEndpoint(string & ip, uint16_t & port) const
	{
		if (state_ == State::Closed)
			return false;

		ip = remote_endpoint_.address().to_string();
		port = remote_endpoint_.port();
		return true;
	}
	inline void TcpSession::Close()
	{
		strand_->dispatch([this] {
			if (!IsOpen())
				return;

			state_ = State::Closed;
			boost::system::error_code ec;
			socket_->shutdown(tcp::socket::shutdown_both, ec);
			socket_->close();

			if (closeHandler)
				closeHandler(CloseReason::ActiveClose);
		});
	}

	inline bool TcpSession::IsOpen() const
	{
		return state_ == State::Opened;
	}

	// 세션을 시작한다.

	inline void TcpSession::Start()
	{
		strand_->dispatch([this] {

			// read 버퍼 생성
			size_t initial_capacity = min_receive_size_;
			size_t max_capacity = max_receive_buffer_size_;
			read_buf_ = std::make_shared<Buffer>(initial_capacity, max_capacity);

			state_ = State::Opened;
			Read(min_receive_size_);

			openHandler();
		});
	}
}


