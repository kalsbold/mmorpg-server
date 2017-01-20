#include "gisunnet/network/tcp/TcpClient.h"

namespace gisunnet {

TcpClient::TcpClient(const ClientConfiguration & config)
	: Client()
	, config_(config)
	, state_(State::Ready)
{
	// ������ io_service_pool �� ������ ����.
	if (config_.io_service_pool.get() == nullptr)
	{
		size_t thread_count = 1;
		config_.io_service_pool = std::make_shared<IoServicePool>(thread_count);
	}
	ios_pool_ = config_.io_service_pool;
	strand_ = std::make_unique<strand>(ios_pool_->PickIoService());
}

TcpClient::~TcpClient()
{
	Close();
}

void TcpClient::Connect(const std::string & host, const std::string & service)
{
	strand_->dispatch([this, host, service]
	{
		if (!(state_ == State::Ready))
			return;

		tcp::resolver resolver(ios_pool_->PickIoService());
		auto endpoint_iterator = resolver.resolve({ host, service });

		ConnectStart(endpoint_iterator);
		state_ = State::Connecting;

		std::cerr << "Connecting to : " << host << ":" << service << "\n";
	});
}

void TcpClient::Close()
{
	strand_->dispatch([this]()
	{
		_Close();
	});
}

void TcpClient::ConnectStart(tcp::resolver::iterator endpoint_iterator)
{
	socket_ = std::make_unique<tcp::socket>(ios_pool_->PickIoService());
	tcp::resolver::iterator end;
	boost::asio::async_connect(*socket_, endpoint_iterator, end, strand_->wrap(
		[this](const error_code& error, tcp::resolver::iterator i)
	{
		if (state_ == State::Closed)
		{
			if (!error)
			{
				socket_->shutdown(tcp::socket::shutdown_both);
				socket_->close();
			}
			socket_.release();
			return;
		}

		if (!error)
		{
			// Start
			// socket option
			socket_->set_option(tcp::no_delay(config_.no_delay));
			// read ���� ����
			size_t initial_capacity = config_.min_receive_size;
			size_t max_capacity = config_.max_receive_buffer_size;
			read_buf_ = std::make_shared<Buffer>(initial_capacity, max_capacity);
			state_ = State::Connected;

			// Read
			Read(config_.min_receive_size);
			net_event_handler_(NetEventType::Opened);
		}
		else
		{
			HandleError(error);
			_Close();
			net_event_handler_(NetEventType::ConnectFailed);
		}
	}));
}

inline void TcpClient::Read(size_t min_read_bytes)
{
	if (!IsConnected())
		return;

	if (!PrepareRead(min_read_bytes))
	{
		_Close();
		net_event_handler_(NetEventType::Closed);
		return;
	}

	auto asio_buf = mutable_buffer(*read_buf_);
	socket_->async_read_some(boost::asio::buffer(asio_buf), strand_->wrap(
		[this](const error_code& error, std::size_t bytes_transferred)
	{
		HandleRead(error, bytes_transferred);
	}));
}

inline void TcpClient::HandleRead(const error_code & error, std::size_t bytes_transferred)
{
	if (!IsConnected())
		return;

	if (error)
	{
		HandleError(error);
		_Close();
		net_event_handler_(NetEventType::Closed);
		return;
	}

	// ������ WriterIndex �� ���۹��� �������� ũ�⸸ŭ ����
	read_buf_->WriterIndex(read_buf_->WriterIndex() + bytes_transferred);

	size_t next_read_size = 0;
	// ���� ����Ÿ ó��
	HandleReceiveData(*read_buf_, next_read_size);

	size_t prepare_size = std::max<size_t>((size_t)next_read_size, config_.min_receive_size);
	Read(prepare_size);
}

inline bool TcpClient::PrepareRead(size_t min_prepare_bytes)
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

inline void TcpClient::PendWrite(Ptr<Buffer>& buf)
{
	if (!IsConnected())
		return;

	pending_list_.emplace_back(std::move(buf));
	if (sending_list_.empty())
	{
		Write();
	}
}

inline void TcpClient::Write()
{
	assert(sending_list_.empty());

	if (!IsConnected())
		return;

	if (pending_list_.empty())
		return;

	// TO DO : sending_list_.swap(pending_list_);
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

	boost::asio::async_write(*socket_, bufs, strand_->wrap(
		[this](error_code const& ec, std::size_t)
	{
		HandleWrite(ec);
	}));
}

inline void TcpClient::HandleWrite(const error_code & error)
{
	if (!IsConnected())
		return;

	if (error)
	{
		HandleError(error);
		_Close();
		net_event_handler_(NetEventType::Closed);
		return;
	}

	sending_list_.clear();
	if (!pending_list_.empty())
	{
		Write();
	}
}

inline void TcpClient::HandleError(const error_code & error)
{
	// ������ ������ �������̴�.
	if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted)
	{
		return;
	}

	std::cerr << "Socket Error : " << error.message() << "\n";
}

void TcpClient::_Close()
{
	if (state_ == State::Closed)
		return;

	boost::system::error_code ec;
	socket_->shutdown(tcp::socket::shutdown_both, ec);
	socket_->close();
	state_ = State::Closed;
}

} // namespace gisunnet