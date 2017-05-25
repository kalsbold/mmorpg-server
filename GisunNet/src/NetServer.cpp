#include "include/network/NetServer.h"
#include "include/network/tcp/TcpServer.h"

namespace gisun {
namespace net {

Ptr<NetServer> NetServer::Create(const ServerConfig& config)
{
	return std::make_shared<TcpServer>(config);
}

} // namespace net
} // namespace gisun