#include "include/network/NetClient.h"
#include "include/network/tcp/TcpClient.h"

namespace gisun {
namespace net {

Ptr<NetClient> NetClient::Create(const ClientConfig& config)
{
	return std::make_shared<TcpClient>(config);
}

} // namespace net
} // namespace gisun