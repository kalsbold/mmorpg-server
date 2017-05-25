#pragma once

#include <thread>
#include "include/Types.h"

namespace gisun {
namespace net {

	class IoServiceLoop;

	struct ServerConfig
	{
	public:
		// execution
		Ptr<IoServiceLoop>  io_service_loop = nullptr;
		std::size_t			thread_count = std::thread::hardware_concurrency(); // io_service_loop 이 설정되어 있으면 이값은 무시.
		// session
		size_t				max_session_count = boost::asio::ip::tcp::acceptor::max_connections;
		size_t				min_receive_size = 1024 * 4;
		size_t				max_receive_buffer_size = std::numeric_limits<size_t>::max();
		// socket options
		bool				no_delay = false;
	};

	struct ClientConfig
	{
	public:
		// execution
		Ptr<IoServiceLoop>	io_service_loop = nullptr;
		// session
		size_t				min_receive_size = 1024 * 4;
		size_t				max_receive_buffer_size = std::numeric_limits<size_t>::max();
		// socket options
		bool				no_delay = false;
	};

} // namespace net
} // namespace gisun