#include "gisunnet/network/NetServer.h"
#include "gisunnet/network/tcp/TcpServer.h"

namespace gisunnet {

Ptr<NetServer> NetServer::Create(const Configuration& config)
{
	return std::make_shared<TcpServer>(config);
}

} // namespace gisunnet