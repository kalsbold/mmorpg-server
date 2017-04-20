#pragma once
#include "Singleton.h"

namespace mmog {

	class ServerConfig : public Singleton<ServerConfig>
	{
	public:
		std::string bind_address;
		uint16_t bind_port;
		size_t thread_count;
		size_t max_session_count;
		bool	no_delay;
		size_t	min_receive_size;
		size_t	max_receive_buffer_size;
		std::string db_host;
		std::string db_user;
		std::string db_password;
		std::string db_schema;
		size_t db_connection_pool;
	};

}