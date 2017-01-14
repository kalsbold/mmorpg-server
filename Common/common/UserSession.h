#pragma once
#include "NetTransport.h"

class UserSession
{
public:
	using TransportPtr = std::shared_ptr<NetTransport>;

	UserSession(TransportPtr transport);
	~UserSession();

private:
	void OnMessage()
	{

	}
	void OnClose()
	{

	}

	void OnError(const std::string& error_msg)
	{

	}

	TransportPtr transport_;
};

UserSession::UserSession(TransportPtr transport)
	: transport_(std::move(transport))
{
	assert(transport_.get() != nullptr);

	transport_->ErrorCallback = [&](const std::string& error_msg) { this->OnError(error_msg); };
	transport_->CloseCallback = [&] { this->OnClose(); };
}

UserSession::~UserSession()
{
}