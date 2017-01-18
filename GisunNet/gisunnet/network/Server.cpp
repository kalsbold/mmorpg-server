#include <gisunnet/network/Server.h>
#include <gisunnet/network/tcp/TcpServer.h>

namespace gisunnet {

	Ptr<Server> Server::Create()
	{
		return Ptr<TcpServer>();
	}

	Server::~Server() {}
	Server::Server() {}
}