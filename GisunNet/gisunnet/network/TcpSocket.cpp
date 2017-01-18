#include <gisunnet/network/TcpSocket.h>
#include <gisunnet/utility/AsioHelper.h>

namespace gisunnet
{

TcpSocket::TcpSocket(std::unique_ptr<tcp::socket> socket)
	: socket_(std::move(socket))
{
	assert(socket_.get() != nullptr);
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::Start()
{
	assert(socket_->is_open() == true);

	//if (start_handler_)
	//	start_handler_();

	// read 버퍼 생성
	size_t initial_capacity = MIN_PREPARE_READ_BYTES;
	size_t max_capacity = MAX_READ_BUF_CAPACITY;
	read_buf_ = std::make_shared<Buffer>(initial_capacity, max_capacity);

	DoRead(MIN_PREPARE_READ_BYTES);
}

void TcpSocket::Close()
{
	if (IsClosed())
		return;

	boost::system::error_code ec;
	socket_->shutdown(tcp::socket::shutdown_both, ec);
	socket_->close();

	if (CloseHandler)
		CloseHandler();
}

bool TcpSocket::PrepareRead(size_t min_prepare_bytes)
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
		std::cerr << "TcpTransport::PrepareRead Exception : " << e.what() << "\n";
		read_buf_->Clear();
		return false;
	}

	return true;
}

void TcpSocket::DoRead(size_t min_read_bytes)
{
	if (IsClosed())
		return;

	if (!PrepareRead(min_read_bytes))
	{
		Close();
		return;
	}

	auto asio_buf = mutable_buffer(*read_buf_);
	auto self = shared_from_this();
	socket_->async_read_some(boost::asio::buffer(asio_buf),
		[this, self](const error_code& error, std::size_t bytes_transferred)
		{
			HandleRead(error, bytes_transferred);
		});
}

void TcpSocket::HandleRead(const error_code & error, std::size_t bytes_transferred)
{
	if (IsClosed())
		return;

	if (error)
	{
		HandleError(error);
		Close();
		return;
	}
	// 버퍼의 WriterIndex 을 전송받은 데이터의 크기만큼 전진
	read_buf_->WriterIndex(read_buf_->WriterIndex() + bytes_transferred);
	
	// 처리
	size_t min_next_read_size = 0;
	if (ReceiveDataHandler)
	{
		ReceiveDataHandler(*read_buf_, min_next_read_size);
	}
	else
	{
		read_buf_->Clear();
	}

	size_t prepare_size = std::max<size_t>((size_t)min_next_read_size, MIN_PREPARE_READ_BYTES);
	DoRead(prepare_size);
}

void TcpSocket::Send(Ptr<Buffer> buffer)
{
	GetIoService().post([this, self = shared_from_this(), buf = std::move(buffer)]() mutable
	{
		PendWrite(buf);
	});
}

void TcpSocket::PendWrite(Ptr<Buffer>& buf)
{
	pending_list_.emplace_back(std::move(buf));
	if (sending_list_.empty())
	{
		DoWrite();
	}
}

void TcpSocket::DoWrite()
{
	assert(sending_list_.empty());

	if (IsClosed())
		return;
	if (pending_list_.empty())
		return;
	
	//sending_list_.swap(pending_list_);
	for (auto& buffer : pending_list_)
	{
		sending_list_.emplace_back(std::move(buffer));
	}
	pending_list_.clear();

	// Scatter-Gather I/O
	std::vector<asio::const_buffer> bufs;
	for (auto& buffer : sending_list_)
	{
		bufs.emplace_back(const_buffer(*buffer));
	}

	auto self = shared_from_this();
	boost::asio::async_write(*socket_, bufs,
		[this, self](error_code const& ec, std::size_t)
		{
			HandleWrite(ec);
		});
}

void TcpSocket::HandleWrite(const error_code& error)
{
	if (IsClosed())
		return;

	if (error)
	{
		HandleError(error);
		Close();
		return;
	}

	sending_list_.clear();
	if (!pending_list_.empty())
	{
		DoWrite();
	}
}

void TcpSocket::HandleError(const error_code& error)
{
	// 상대방이 연결을 끊은것이다.
	if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted)
	{
		return;
	}

	if (ErrorHandler)
		ErrorHandler(error);
}

} // namespace gisunnet
