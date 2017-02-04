#pragma once

#include <tuple>
#include "gisunnet/types.h"
#include "gisunnet/network/Session.h"
#include "gisunnet/network/Configuration.h"
#include "gisunnet/utility/AsioHelper.h"

namespace gisunnet {

class TcpSession : public Session, public std::enable_shared_from_this<TcpSession>
{
public:
	using tcp = boost::asio::ip::tcp;

	TcpSession(const TcpSession&) = delete;
	TcpSession& operator=(const TcpSession&) = delete;

	TcpSession(std::unique_ptr<tcp::socket> socket, const uuid& id, const Configuration& config);

	virtual ~TcpSession();

	virtual const uuid& ID() const override;

	virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const override;

	virtual void Close() override;

	virtual bool IsOpen() const override;

	virtual void Send(const uint8_t * data, size_t size) override
	{
		auto buffer = std::make_shared<Buffer>(size);
		buffer->WriteBytes(data, size);

		Send(std::move(buffer));
	}
	virtual void Send(const Buffer& data) override
	{
		auto buffer = std::make_shared<Buffer>(data);

		Send(std::move(buffer));
	}
	virtual void Send(Ptr<Buffer> message) override
	{
		PendWrite(std::move(message));
	}

	// 세션을 시작한다.
	void Start();

	function<void()> openHandler;
	function<void(CloseReason reason)> closeHandler;
	function<void(uint8_t*, size_t)> recvHandler;

private:
	enum State
	{
		Ready,
		Opening,
		Opened,
		Closed
	};
		
	struct Header
	{
		uint16_t payload_len;
	};

	using SendMsg = std::tuple<Buffer, Ptr<Buffer>>;
	using strand = boost::asio::io_service::strand;

	void Read(size_t min_read_bytes);
	void HandleRead(const error_code & error, std::size_t bytes_transferred);
	bool PrepareRead(size_t min_prepare_bytes);

	// Parse Message
	void HandleReceiveData(Buffer& read_buf, size_t& next_read_size)
	{
		DecodeRecvData(read_buf, next_read_size);
	}

	void PendWrite(Ptr<Buffer> buf);
	void Write();
	void HandleWrite(const error_code& error);
	void HandleError(const error_code& error);
	void _Close(CloseReason reason);

	void EncodeSendData(Ptr<Buffer>& data)
	{
		// Make Header
		Header header;
		header.payload_len = data->ReadableBytes();

		// To Do : 암호화나 압축 등..
		
		data->InsertBytes(data->ReaderIndex(), reinterpret_cast<uint8_t*>(&header.payload_len), 0, sizeof(header.payload_len));
		data->WriterIndex(data->WriterIndex() + sizeof(header.payload_len));
	}

	void DecodeRecvData(Buffer& buf, size_t&)
	{
		Header header = {0};
		// 처리할 데이터가 있으면 반복
		while (buf.IsReadable())
		{
			// Decode Header
			// 헤더 사이즈 만큼 받지 못했으면 리턴
			if (!buf.IsReadable(sizeof(Header)))
				return;

			// TO DO : 헤더 검증?
			buf.GetPOD(buf.ReaderIndex(), header.payload_len);

			// Decode Body
			// Header + Body 사이즈 만큼 받지 못했으면 리턴
			if (!buf.IsReadable(sizeof(Header) + header.payload_len))
				return;

			// 헤더 사이즈만큼 전진
			buf.SkipBytes(sizeof(Header));
			// TO DO : 복호화?
			
			// Call receive handler
			if (recvHandler)
				recvHandler(buf.Data() + buf.ReaderIndex(), header.payload_len);
			
			// 바디 사이즈만큼 전진
			buf.SkipBytes(header.payload_len);
		}
	}

	std::unique_ptr<tcp::socket> socket_;
	std::unique_ptr<strand> strand_;
	tcp::endpoint remote_endpoint_;
		
	uuid id_;
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



