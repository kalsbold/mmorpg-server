#pragma once

#include "gisunnet/network/Client.h"
#include "gisunnet/network/IoServicePool.h"
#include "gisunnet/utility/AsioHelper.h"

namespace gisunnet {

class TcpClient : public Client
{
public:
	using tcp = boost::asio::ip::tcp;

	TcpClient(const ClientConfiguration& config);
	~TcpClient();

	virtual void Connect(const std::string& host, const std::string& service) override;
	virtual void Close() override;
	virtual bool IsConnected() const override
	{
		return state_ == State::Connected;
	}

	virtual void Send(Ptr<Buffer> message) override
	{
		strand_->dispatch([this, message = std::move(message)]() mutable { PendWrite(message); });
	}

	virtual void RegisterNetEventHandler(const NetEventHandler& handler) override
	{
		net_event_handler_ = handler;
	}
	virtual void RegisterMessageHandler(uint16_t message_type, const MessageHandler& handler) override
	{
		message_handler_map_.emplace(message_type, handler);
	}

private:
	enum class State
	{
		Ready = 0,
		Connecting,
		Connected,
		Closed,
	};

	using strand = boost::asio::io_service::strand;
	using MessageHandlerMap = std::map<uint16_t, MessageHandler>;

	void ConnectStart(tcp::resolver::iterator endpoint_iterator);
	void Read(size_t min_read_bytes);
	void HandleRead(const error_code & error, std::size_t bytes_transferred);

	void HandleReceiveData(Buffer& read_buf, size_t&)
	{
		// To Do : 
		string str(read_buf.Data() + read_buf.ReaderIndex(), read_buf.Data() + read_buf.WriterIndex());
		std::cout << str << std::endl;
		read_buf.Clear();
	}

	bool PrepareRead(size_t min_prepare_bytes);
	void PendWrite(Ptr<Buffer>& buf);
	void Write();
	void HandleWrite(const error_code & error);
	void HandleError(const error_code & error);
	void _Close();

	NetEventHandler net_event_handler_;
	MessageHandlerMap message_handler_map_;

	std::unique_ptr<tcp::socket> socket_;
	std::unique_ptr<strand> strand_;
	
	Ptr<Buffer> read_buf_;
	std::vector<Ptr<Buffer>> pending_list_;
	std::vector<Ptr<Buffer>> sending_list_;

	ClientConfiguration	config_;
	State				state_;
	Ptr<IoServicePool>	ios_pool_;
	//std::mutex			mutex_;
	
};

} // namespace gisunnet