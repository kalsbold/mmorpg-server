#include "TcpTransport.h"
#include "AsioHelper.h"

namespace gisunnet
{

TcpTransport::TcpTransport(std::unique_ptr<tcp::socket> socket)
	: socket_(std::move(socket))
{
	assert(socket_.get() != nullptr);
}

TcpTransport::~TcpTransport()
{
}

void TcpTransport::Start()
{
	assert(socket_->is_open() == true);

	//if (start_handler_)
	//	start_handler_();

	// read ���� ����
	size_t initial_capacity = MIN_PREPARE_READ_BYTES;
	size_t max_capacity = MAX_READ_BUF_CAPACITY;
	read_buf_ = std::make_unique<ByteBuf>(initial_capacity, max_capacity);

	DoRead(MIN_PREPARE_READ_BYTES);
}

void TcpTransport::Close()
{
	if (IsClosed())
		return;

	boost::system::error_code ec;
	socket_->shutdown(tcp::socket::shutdown_both, ec);
	socket_->close();

	if (CloseCallback)
		CloseCallback();
}

bool TcpTransport::PrepareRead(size_t min_prepare_bytes)
{
	// ������ ������ �������� �������ش�.
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

void TcpTransport::DoRead(size_t min_read_bytes)
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

void TcpTransport::HandleRead(const error_code & error, std::size_t bytes_transferred)
{
	if (IsClosed())
		return;

	if (error)
	{
		HandleError(error);
		Close();
		return;
	}
	// ������ WriterIndex �� ���۹��� �������� ũ�⸸ŭ ����
	read_buf_->WriterIndex(read_buf_->WriterIndex() + bytes_transferred);
	
	// ó��
	size_t min_next_read_size = 0;
	if (ReadCallback)
	{
		ReadCallback(*read_buf_, min_next_read_size);
	}
	else
	{
		read_buf_->Clear();
	}

	size_t prepare_size = std::max<size_t>((size_t)min_next_read_size, MIN_PREPARE_READ_BYTES);
	DoRead(prepare_size);
}

void TcpTransport::Write(std::shared_ptr<ByteBuf> buffer)
{
	GetIoService().post([this, self = shared_from_this(), buf = std::move(buffer)]() mutable
	{
		_Write(buf);
	});
}

void TcpTransport::_Write(std::shared_ptr<ByteBuf>& buf)
{
	pending_list_.emplace_back(std::move(buf));
	if (sending_list_.empty())
	{
		DoWrite();
	}
}

void TcpTransport::DoWrite()
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

void TcpTransport::HandleWrite(const error_code& error)
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

void TcpTransport::HandleError(const error_code& error)
{
	// ������ ������ �������̴�.
	if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted)
	{
		return;
	}

	if (ErrorCallback)
		ErrorCallback(error);
}

} // namespace gisunnet
