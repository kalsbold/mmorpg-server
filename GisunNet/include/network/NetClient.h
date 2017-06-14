#pragma once

#include "Types.h"
#include "network/NetConfig.h"
#include "network/ByteBuffer.h"

namespace gisun {
namespace net {

	enum class NetEventType : int
	{
		Opened,
		Closed,
		//Changed,
		ConnectFailed,
	};

	class NetClient
	{
	public:
		// Network evnet handler.
		using NetEventHandler = std::function<void(const NetEventType&)>;
		// Message handler.
		using MessageHandler = std::function<void(const uint8_t*, size_t)>;

		// Create client instance.
		static Ptr<NetClient> Create(const ClientConfig& config);

		NetClient(const NetClient&) = delete;
		NetClient& operator=(const NetClient&) = delete;

		virtual ~NetClient() {};

		virtual void Connect(const std::string& host, const std::string& service) = 0;
		virtual void Close() = 0;
		virtual bool IsConnected() const = 0;
		virtual void Send(const uint8_t* data, size_t size) = 0;
		virtual void Send(const Buffer& data) = 0;
		virtual void Send(Buffer&& data) = 0;
		virtual void Send(Ptr<Buffer> data) = 0;
		virtual void RegisterNetEventHandler(const NetEventHandler& handler) = 0;
		virtual void RegisterMessageHandler(const MessageHandler& handler) = 0;

		virtual Ptr<IoServiceLoop> GetIoServiceLoop() = 0;
		virtual asio::strand& GetStrand() = 0;

	protected:
		NetClient() {};
	};

} // namespace net
} // namespace gisun