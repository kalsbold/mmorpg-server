#pragma once

#include <gisunnet/types.h>
#include <gisunnet/network/Session.h>

namespace gisunnet
{

	class TcpSession : public Session
	{
	public:
		using tcp = boost::asio::ip::tcp;
		using SessionID = uuid;

		TcpSession(const SessionID& id, std::unique_ptr<tcp::socket> socket)
			: id_(id)
			, socket_(std::move(socket))
		{
			assert(socket_.get() != nullptr);
			assert(socket_->is_open());

			ios_ = &(socket_->get_io_service());
		}

		virtual ~TcpSession()
		{

		}

		// Inherited via Session
		virtual const SessionID& ID() const override;
		virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const override;
		virtual void Close() override;
		virtual bool IsOpen() const override;
		virtual void SendMessage(const string&message_type, const Ptr<Buffer>& message);

		// 세션을 시작한다.
		void Start()
		{
			ios_->dispatch([this]()
			{
				openHandler();
			});
		}

	private:
		SessionID id_;
		boost::asio::io_service* ios_ = nullptr;
		std::unique_ptr<tcp::socket> socket_;
	};

}



