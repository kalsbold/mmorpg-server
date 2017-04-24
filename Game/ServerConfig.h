#pragma once
#include "Singleton.h"

namespace mmog {

	class ServerConfig : public Singleton<ServerConfig>
	{
	public:
		string   server_name;
		string   bind_address;
		uint16_t bind_port;
		size_t   thread_count;
		size_t   max_session_count;
		bool	 no_delay;
		size_t	 min_receive_size;
		size_t	 max_receive_buffer_size;
		string   db_host;
		string   db_user;
		string   db_password;
		string   db_schema;
		size_t   db_connection_pool;

		static void LoadFile()
		{

		}
	};

}