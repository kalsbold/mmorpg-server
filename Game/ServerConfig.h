#pragma once
#include "Singleton.h"

namespace mmog {

	class ServerConfig : public Singleton<ServerConfig>
	{
	public:
		std::string bind_address = "0.0.0.0";
		int bind_port;
		size_t thread_count = 1;
		size_t max_session_count = 1000;
		std::string db_host;
		std::string db_user;
		std::string db_password;
		std::string db_schema;
		size_t db_connection_pool = 1;
	};

}