#pragma once

#include <thread>
#include <gisunnet/types.h>

namespace gisunnet {

class IoServicePool;

struct Configuration
{
public:
	// execution config
	Ptr<IoServicePool>	io_service_pool = nullptr;
	std::size_t			thread_count = std::thread::hardware_concurrency();
	// session manager config
	size_t				max_session_count = boost::asio::ip::tcp::acceptor::max_connections;
	// session config
	bool	no_delay = false;
	size_t	min_receive_size = 1024 * 4;
	size_t	max_receive_buffer_size = std::numeric_limits<size_t>::max();
	size_t	max_transfer_size = std::numeric_limits<size_t>::max();
};

}