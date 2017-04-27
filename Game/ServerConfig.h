#pragma once
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <boost\program_options.hpp>
#include "Singleton.h"

namespace mmog {

	using namespace std;
	namespace po = boost::program_options;

	class ServerConfig : public Singleton<ServerConfig>
	{
	public:
		string   name;
		string   bind_address;
		uint16_t bind_port;
		size_t   thread_count;
		size_t   max_session_count;
		size_t	 min_receive_size;
		size_t	 max_receive_buffer_size;
		bool	 no_delay;
		string   db_host;
		string   db_user;
		string   db_password;
		string   db_schema;
		size_t   db_connection_pool;

		template <typename CharT>
		static bool Load(CharT* filepath)
		{
			ServerConfig& config = ServerConfig::GetInstance();

			// step 1 : 옵션 설명 정의
			po::options_description desc("Allowed options");
			desc.add_options()
				("Server.name", po::value<string>(&config.name)->default_value(""))
				("Server.address", po::value<string>(&config.bind_address)->default_value("0.0.0.0"))
				("Server.port", po::value<uint16_t>())
				("Server.thread", po::value<size_t>(&config.thread_count)->default_value(thread::hardware_concurrency()))
				("Server.max-session", po::value<size_t>(&config.max_session_count)->default_value(10000))
				("Server.min-receive-size", po::value<size_t>(&config.min_receive_size)->default_value(1024 * 4))
				("Server.max-buffer-size", po::value<size_t>(&config.max_receive_buffer_size)->default_value(numeric_limits<size_t>::max()))
				("Server.no-delay", po::value<bool>(&config.no_delay)->default_value(false))
				("DB.host", po::value<string>())
				("DB.user", po::value<string>())
				("DB.password", po::value<string>())
				("DB.schema", po::value<string>())
				("DB.conn-pool", po::value<size_t>(&config.db_connection_pool)->default_value(1))
				;

			// step 2 :명령행 옵션 분석
			po::variables_map vm;
			try
			{
				std::ifstream ifs(filepath);
				if (!ifs.is_open())
				{
					std::cerr << "Can not open file: " << filepath << "\n";
					return false;
				}
				po::store(po::parse_config_file(ifs, desc), vm);
			}
			catch (po::unknown_option & e)
			{
				// 예외 1: 없는 옵션을 사용한 경우
				std::cerr << e.what() << "\n";
				return false;
			}
			catch (po::invalid_option_value & e)
			{
				// 예외 2: 옵션 값의 오류가 발생한 경우
				std::cerr << e.what() << std::endl;
				return false;
			}
			catch (po::invalid_command_line_syntax & e)
			{
				// 예외 3: 옵션 값이 없을 경우
				std::cerr << e.what() << "\n";
				return false;
			}
			catch (std::exception & e)
			{
				// 예외 4: 이외의 예외 발생
				std::cerr << e.what() << "\n";
				return false;
			}

			po::notify(vm);

			// step 3 : 옵션 처리
			if (vm.count("Server.port"))
			{
				config.bind_port = vm["Server.port"].as<uint16_t>();
			}
			else
			{
				std::cerr << "Server.port required" << "\n";
				return false;
			}

			if (vm.count("DB.host"))
			{
				config.db_host = vm["DB.host"].as<string>();
			}
			else
			{
				std::cerr << "DB.host required" << "\n";
				return false;
			}

			if (vm.count("DB.user"))
			{
				config.db_user = vm["DB.user"].as<string>();
			}
			else
			{
				std::cerr << "DB.user required" << "\n";
				return false;
			}

			if (vm.count("DB.password"))
			{
				config.db_password = vm["DB.password"].as<string>();
			}
			else
			{
				std::cerr << "DB.pass required" << "\n";
				return false;
			}

			if (vm.count("DB.schema"))
			{
				config.db_schema = vm["DB.schema"].as<string>();
			}
			else
			{
				std::cerr << "DB.schema required" << "\n";
				return false;
			}
			return true;
		}
	};

}