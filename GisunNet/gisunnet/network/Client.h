#pragma once

#include "gisunnet/types.h"
#include "gisunnet/network/Configuration.h"

namespace gisunnet {

enum class NetEventType : int
{
	Opened,
	Closed,
	//Changed,
	ConnectFailed,
};

class Client
{
public:
	// Network evnet handler.
	using NetEventHandler = function<void(const NetEventType&)>;
	// Message handler.
	using MessageHandler = function<void(const uint8_t*, size_t)>;

	// Create client instance.
	static Ptr<Client> Create(const ClientConfiguration& config);

	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;
		
	virtual ~Client() {};

	virtual void Connect(const std::string& host, const std::string& service) = 0;
	virtual void Close() = 0;
	virtual bool IsConnected() const = 0;
	virtual void Send(const uint8_t* data, size_t size) = 0;
	virtual void Send(const Buffer& data) = 0;
	virtual void Send(Ptr<Buffer> message) = 0;
	virtual void RegisterNetEventHandler(const NetEventHandler& handler) = 0;
	virtual void RegisterMessageHandler(const MessageHandler& handler) = 0;
	
protected:
	Client() {};
};
	
} // namespace gisunnet