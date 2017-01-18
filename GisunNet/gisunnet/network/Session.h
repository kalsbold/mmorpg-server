#pragma once

#include <memory>
#include <gisunnet/types.h>
#include <gisunnet/network/Server.h>

namespace gisunnet{

	class Session
	{
	public:
		using SessionID = uuid;
		
		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;

		virtual ~Session();

		virtual const SessionID& ID() const = 0;

		virtual bool GetRemoteEndpoint(string& ip, uint16_t& port) const = 0;

		//bool IsSocketAttached() const;

		//void CloseSocket();

		virtual void SendMessage(const string& message_type, const Ptr<Buffer>& message) = 0;

		virtual void Close() = 0;

		virtual bool IsOpen() const = 0;

		//void SetPingTimeout();

		function<void()> openHandler;
		function<void(CloseReason reason)> closeHandler;

	protected:
		Session();
	};

	Session::Session() {}
	Session::~Session(){}
}



