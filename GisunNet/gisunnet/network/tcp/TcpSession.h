#pragma once

#include <gisunnet/types.h>
#include <gisunnet/network/Session.h>
#include <gisunnet/network/Configuration.h>
#include <gisunnet/utility/AsioHelper.h>

namespace gisunnet
{

	class TcpSession : public Session
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using SessionID = uuid;

		TcpSession(const SessionID& id, std::unique_ptr<tcp::socket> socket, const Configuration& config)
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

		virtual ~TcpSession()
		{

		}

		// Inherited via Session
		virtual const SessionID& ID() const override
		{
			return id_;
		}

		virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const override
		{
			if (state_ == State::Closed)
				return false;

			ip = remote_endpoint_.address().to_string();
			port = remote_endpoint_.port();
			return true;
		}

		virtual void Close() override
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

		virtual bool IsOpen() const override
		{
			return state_ == State::Opened;
		}

		virtual void SendMessage(const string&message_type, const Ptr<Buffer>& message)
		{

		}

		// 세션을 시작한다.
		void Start()
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

		function<void()> openHandler;
		function<void(CloseReason reason)> closeHandler;

	private:
		using strand = boost::asio::io_service::strand;

		enum State
		{
			Opening,
			Opened,
			Closed
		};
		
		struct Header
		{
			int16_t payload_length;
			int16_t message_type;
		};

		void Read(size_t min_read_bytes)
		{
			if (!IsOpen())
				return;

			if (!PrepareRead(min_read_bytes))
			{
				Close();
				return;
			}

			auto asio_buf = mutable_buffer(*read_buf_);
			socket_->async_read_some(boost::asio::buffer(asio_buf), strand_->wrap(
				[this](const error_code& error, std::size_t bytes_transferred)
				{
					HandleRead(error, bytes_transferred);
				}));
		}

		void HandleRead(const error_code & error, std::size_t bytes_transferred)
		{
			if (!IsOpen())
				return;

			if (error)
			{
				Close();
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

		bool PrepareRead(size_t min_prepare_bytes)
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

		// Parse Message
		size_t HandleReceiveData(Buffer& read_buf, size_t& next_read_size)
		{

		}

		std::unique_ptr<tcp::socket> socket_;
		std::unique_ptr<strand> strand_;
		tcp::endpoint remote_endpoint_;
		
		SessionID id_;
		State state_;

		Ptr<Buffer> read_buf_;
		std::vector<Ptr<Buffer>> pending_list_;
		std::vector<Ptr<Buffer>> sending_list_;

		// config
		bool	no_delay_ = false;
		size_t	min_receive_size_;
		size_t	max_receive_buffer_size_;
		size_t	max_transfer_size_;
	};

}



