#include "network/tcp/TcpSession.h"

namespace gisun {
namespace net {

TcpSession::TcpSession(std::unique_ptr<tcp::socket> socket, int id, const ServerConfig & config)
	: socket_(std::move(socket))
	, id_(id)
	, state_(State::Ready)
{
	assert(socket_.get() != nullptr);
	assert(socket_->is_open());

	strand_ = std::make_unique<asio::strand>(socket_->get_io_service());
	remote_endpoint_ = socket_->remote_endpoint();

	// Set config
	no_delay_ = config.no_delay;
	min_receive_size_ = config.min_receive_size;
	max_receive_buffer_size_ = config.max_receive_buffer_size;
}

TcpSession::~TcpSession()
{
}

int TcpSession::GetID() const
{
	return id_;
}

bool TcpSession::GetRemoteEndpoint(std::string& ip, uint16_t& port) const
{
	if (state_ == State::Closed)
		return false;

	ip = remote_endpoint_.address().to_string();
	port = remote_endpoint_.port();
	return true;
}

void TcpSession::Close()
{
	strand_->post([this, self = shared_from_this()]
	{
		_Close(CloseReason::ActiveClose);
	});
}

bool TcpSession::IsOpen() const
{
	return state_ == State::Opened;
}

void TcpSession::Start()
{
	strand_->post([this, self = shared_from_this()]
	{
		try
		{
			state_ = State::Opening;

			// socket option
			socket_->set_option(tcp::no_delay(no_delay_));

			// read 버퍼 생성
			size_t initial_capacity = min_receive_size_;
			size_t max_capacity = max_receive_buffer_size_;
			read_buf_ = std::make_shared<Buffer>(initial_capacity, max_capacity);
		}
		catch (const std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "TcpSession socket set option exception : " << e.what();
			_Close(CloseReason::ActiveClose);
			return;
		}

		state_ = State::Opened;
		Read(min_receive_size_);

		open_handler(shared_from_this());
	});
}

inline void TcpSession::Read(size_t min_read_bytes)
{
	if (!IsOpen())
		return;

	if (!PrepareRead(min_read_bytes))
	{
		_Close(CloseReason::ActiveClose);
		return;
	}

	auto asio_buf = mutable_buffer(*read_buf_);
	socket_->async_read_some(boost::asio::buffer(asio_buf), strand_->wrap(
		[this, self = shared_from_this()](const error_code& error, std::size_t bytes_transferred)
	{
		HandleRead(error, bytes_transferred);
	}));
}

inline void TcpSession::HandleRead(const error_code & error, std::size_t bytes_transferred)
{
	if (!IsOpen())
		return;

	if (error)
	{
		CloseReason reason = CloseReason::ActiveClose;
		if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted)
			reason = CloseReason::Disconnected;

		HandleError(error);
		_Close(reason);
		return;
	}

	// 버퍼의 WriterIndex 을 전송받은 데이터의 크기만큼 전진
	read_buf_->WriterIndex(read_buf_->WriterIndex() + bytes_transferred);

	size_t next_read_size = 0;
	// 받은 데이타 처리
	HandleReceiveData(*read_buf_, next_read_size);

	size_t prepare_size = std::max<size_t>((size_t)next_read_size, min_receive_size_);
	Read(prepare_size);
}

inline bool TcpSession::PrepareRead(size_t min_prepare_bytes)
{
	// 읽을게 없으면 포지션을 리셋해준다.
	if (read_buf_->ReadableBytes() == 0)
	{
		read_buf_->Clear();
	}

	try
	{
		if (read_buf_->WritableBytes() < min_prepare_bytes)
		{
			read_buf_->DiscardReadBytes();
		}
		read_buf_->EnsureWritable(min_prepare_bytes);
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(warning) << "TcpSession prepare read exception : " << e.what() << "\n";
		read_buf_->Clear();
		return false;
	}

	return true;
}

inline void TcpSession::PendWrite(const Ptr<Buffer>& buf)
{
	strand_->dispatch([this, self = shared_from_this(), buf = buf]() mutable
	{
		if (!IsOpen())
			return;

		pending_list_.emplace_back(buf);
		if (sending_list_.empty())
		{
			Write();
		}
	});
}

inline void TcpSession::Write()
{
	assert(sending_list_.empty());

	if (!IsOpen())
		return;

	if (pending_list_.empty())
		return;

	// TO DO : sending_list_.swap(pending_list_);

	// Scatter-Gather I/O
	std::vector<asio::const_buffer> bufs;
	for (auto& buffer : pending_list_)
	{
		EncodeSendData(buffer);
		bufs.emplace_back(const_buffer(*buffer));
		sending_list_.emplace_back(std::move(buffer));
	}
	pending_list_.clear();

	boost::asio::async_write(*socket_, bufs, strand_->wrap(
		[this, self = shared_from_this()](error_code const& ec, std::size_t)
		{
			HandleWrite(ec);
		}));
}

inline void TcpSession::HandleWrite(const error_code & error)
{
	if (!IsOpen())
		return;

	if (error)
	{
		HandleError(error);
		_Close(CloseReason::ActiveClose);
		return;
	}

	sending_list_.clear();
	if (!pending_list_.empty())
	{
		Write();
	}
}

void TcpSession::HandleError(const error_code & error)
{
	// 상대방이 연결을 끊은것이다.
	if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted)
	{
		return;
	}

	BOOST_LOG_TRIVIAL(info) << "TcpSession socket error : " << error.message();
}

void TcpSession::_Close(CloseReason reason)
{
	if (state_ == State::Closed)
		return;

	boost::system::error_code ec;
	socket_->shutdown(tcp::socket::shutdown_both, ec);
	socket_->close();
	state_ = State::Closed;

	if (close_handler)
		close_handler(shared_from_this(), reason);
}

} // namespace net
} // namespace gisun