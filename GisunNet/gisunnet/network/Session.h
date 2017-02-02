#pragma once

#include "gisunnet/types.h"

namespace gisunnet {

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

	virtual const uuid& ID() const = 0;

	virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const = 0;

	//bool IsSocketAttached() const;

	//void CloseSocket();

	virtual void Send(const uint8_t* data, size_t size) = 0;

	virtual void Send(const Buffer& data) = 0;

	virtual void Send(Ptr<Buffer> data) = 0;

	virtual void Close() = 0;

	virtual bool IsOpen() const = 0;

	//void SetPingTimeout();

protected:
	Session() {};
};

} // namespace gisunnet



