#include "gisunnet/network/Client.h"
#include "gisunnet/network/tcp/TcpClient.h"

namespace gisunnet {

	Ptr<Client> Client::Create(const ClientConfiguration& config)
	{
		return std::make_shared<TcpClient>(config);
	}

} // namespace gisunnet