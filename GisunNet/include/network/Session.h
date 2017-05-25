#pragma once

#include "include/Types.h"
#include "include/network/ByteBuffer.h"

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

		virtual bool GetRemoteEndpoint(std::string& ip, uint16_t& port) const = 0;

		//bool IsSocketAttached() const;

		//void CloseSocket();

		virtual void Send(const uint8_t* data, size_t size) = 0;

		virtual void Send(const Buffer& data) = 0;

		virtual void Send(Ptr<Buffer> data) = 0;

		virtual void Close() = 0;

		virtual bool IsOpen() const = 0;

		//void SetPingTimeout();

		virtual asio::strand& GetStrand() = 0;

	protected:
		Session() {};
	};

	template <typename Func>
	void Post(Session& session, Func&& f)
	{
		session.GetStrand().post(std::forward<Func>(f));
	}

	template <typename Func>
	void Dispatch(Session& session, Func&& f)
	{
		session.GetStrand().dispatch(std::forward<Func>(f));
	}

} // namespace net
} // namespace gisun



