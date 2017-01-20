#pragma once

#include "gisunnet/types.h"
#include "gisunnet/network/Session.h"
#include "gisunnet/network/Configuration.h"
#include "gisunnet/utility/AsioHelper.h"

namespace gisunnet {

class TcpSession : public Session
{
public:
	using tcp = boost::asio::ip::tcp;
	using SessionID = uuid;

	TcpSession(const TcpSession&) = delete;
	TcpSession& operator=(const TcpSession&) = delete;

	TcpSession(std::unique_ptr<tcp::socket> socket, const SessionID& id, const Configuration& config);

	virtual ~TcpSession();

	virtual const SessionID& ID() const override;

	virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const override;

	virtual void Close() override;

	virtual bool IsOpen() const override;

	virtual void Send(Ptr<Buffer> message) override
	{
		strand_->dispatch([this, message = std::move(message)]() mutable
		{
			PendWrite(message);
		});
	}

	// 세션을 시작한다.
	void Start();

	function<void()> openHandler;
	function<void(CloseReason reason)> closeHandler;

private:
	using strand = boost::asio::io_service::strand;

	enum State
	{
		Ready,
		Opening,
		Opened,
		Closed
	};
		
	struct Header
	{
		uint16_t payload_length;
		uint16_t message_type;
	};

	void Read(size_t min_read_bytes);
	void HandleRead(const error_code & error, std::size_t bytes_transferred);
	bool PrepareRead(size_t min_prepare_bytes);

	// Parse Message
	void HandleReceiveData(Buffer& read_buf, size_t& next_read_size)
	{
		// To Do : 
		string str(read_buf.Data() + read_buf.ReaderIndex(), read_buf.Data() + read_buf.WriterIndex());
		std::cout << str << std::endl;
		Ptr<Buffer> buf = std::make_shared<Buffer>(read_buf);
		Send(std::move(buf));
		read_buf.Clear();
	}

	void PendWrite(Ptr<Buffer>& buf);
	void Write();
	void HandleWrite(const error_code& error);
	void HandleError(const error_code& error);
	void _Close(CloseReason reason);

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

} // namespace gisunnet



