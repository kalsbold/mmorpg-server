#pragma once
#include <memory>
#include <boost/uuid/uuid.hpp>
#include "TcpTransport.h"
#include "ByteBuffer.h"

namespace gisunnet
{

	class NetSession
	{
	public:
		using UID = boost::uuids::uuid;
		using TcpTransportPtr = std::shared_ptr<TcpTransport>;

		NetSession(const UID& uid, TcpTransportPtr transport)
			: uid_(uid)
			, transport_(std::move(transport))
		{
			transport_->CloseHandler = [this]() { HandleClose(); };
			transport_->ErrorHandler = [this](const TcpTransport::error_code& error)
				{
					std::cerr << error.message() << "\n";
				};
			transport_->ReceiveDataHandler = [this](Buffer& buffer, size_t& next_read_bytes)
				{ HandleReceiveData(buffer, next_read_bytes); };
		}
		~NetSession()
		{

		}

		void Stop()
		{
			transport_->Close();
		}

		const UID& GetUID() const
		{
			return uid_;
		}

		std::function<void(const UID& uid)> StopHandler;

	private:
		void HandleClose()
		{
			if (StopHandler)
				StopHandler(uid_);
		}

		void HandleReceiveData(Buffer& buffer, size_t& next_read_bytes)
		{

		}

		UID uid_;
		TcpTransportPtr transport_;
	};

}



