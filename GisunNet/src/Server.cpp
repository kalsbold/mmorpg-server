#include "gisunnet/network/Server.h"
#include "gisunnet/network/tcp/TcpServer.h"

namespace gisunnet {

Ptr<Server> Server::Create(const Configuration& config)
{
	return std::make_shared<TcpServer>(config);
}

} // namespace gisunnet