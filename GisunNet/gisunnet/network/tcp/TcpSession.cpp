#include <gisunnet/network/tcp/TcpSession.h>


namespace gisunnet {

	const SessionID& TcpSession::ID() const
	{
		return id_;
	}

	bool TcpSession::GetRemoteEndpoint(string & ip, uint16_t & port) const
	{
		return false;
	}

	void TcpSession::Close()
	{
	}

	bool TcpSession::IsOpen() const
	{
		return false;
	}

	void TcpSession::SendMessage(const gisunnet::string &message_type, const gisunnet::Ptr<gisunnet::Buffer> &message)
	{
	}

}


