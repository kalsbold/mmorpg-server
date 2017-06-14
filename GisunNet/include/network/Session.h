#pragma once

#include "Types.h"
#include "network/ByteBuffer.h"

namespace gisun {
namespace net {

	enum CloseReason {
		ActiveClose = 0,	// 서버에서 연결을 끊음.
		Disconnected,		// 클라이언트에서 연결이 끊김.
		Timeout,			//
	};

	class Session
	{
	public:
		DECLARE_CLASS_PTR(Session)

		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;

		virtual ~Session() {};

		virtual int GetID() const = 0;

		virtual bool GetRemoteEndpoint(std::string& ip, uint16_t& port) const = 0;

		//bool IsSocketAttached() const;

		//void CloseSocket();

		virtual void Send(const uint8_t* data, size_t size) = 0;

		virtual void Send(const Buffer& data) = 0;

		virtual void Send(Buffer&& data) = 0;

		virtual void Send(Ptr<Buffer> data) = 0;

		virtual void Close() = 0;

		virtual bool IsOpen() const = 0;

		//void SetPingTimeout();

		virtual asio::strand& GetStrand() = 0;

	protected:
		Session() {};
	};

} // namespace net
} // namespace gisun



