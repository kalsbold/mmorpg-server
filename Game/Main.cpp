// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost\program_options.hpp>
#include "ServerConfig.h"
#include "GameServer.h"
#include "EventHandlers.h"

using namespace std;
using namespace mmog;

int SetServerConfig(int argc, _TCHAR* argv[])
{
	namespace po = boost::program_options;
	
	ServerConfig& config = ServerConfig::GetInstance();
	//config.bind_port = 8088;
	//config.thread_count = 4;
	//config.db_host = "127.0.0.1:3306";
	//config.db_user = "nusigmik";
	//config.db_password = "56561163";
	//config.db_schema = "Project_MMOG";
	//config.db_connection_pool = 4;

	// �ɼ� ���� ����
	po::options_description desc("Configuration");
	desc.add_options()
		("help", "produce help message")
		("address", po::value<string>(&config.bind_address)->default_value("0.0.0.0"))
		("port", po::value<uint16_t>())
		("thread", po::value<size_t>(&config.thread_count)->default_value(thread::hardware_concurrency()))
		("max_session", po::value<size_t>(&config.max_session_count)->default_value(10000))
		("no-delay", po::value<bool>(&config.no_delay)->default_value(false))
		("min-receive-size", po::value<size_t>(&config.min_receive_size)->default_value(1024 * 4))
		("max-buffer-size", po::value<size_t>(&config.max_receive_buffer_size)->default_value(numeric_limits<size_t>::max()))
		("db-host", po::value<string>())
		("db-user", po::value<string>())
		("db-pass", po::value<string>())
		("db-schema", po::value<string>())
		("db-conn-pool", po::value<size_t>(&config.db_connection_pool)->default_value(1))
		;

	// �ɼ��� ���� ���
	if (2 > argc)
	{
		std::cout << desc << "\n";
		return 1;
	}

	// ����� �ɼ� �м�
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch (po::unknown_option & e)
	{
		// ���� 1: ���� �ɼ��� ����� ���
		std::cout << e.what() << std::endl;
		return 10;
	}
	catch (po::invalid_option_value & e)
	{
		// ���� 2: �ɼ� ���� ������ �߻��� ���
		std::cout << e.what() << std::endl;
		return 11;
	}
	catch (po::invalid_command_line_syntax & e)
	{
		// ���� 3: �ɼ� ���� ���� ���
		std::cout << e.what() << std::endl;
		return 12;
	}
	catch (std::exception & e)
	{
		// ���� 4: �̿��� ���� �߻�
		std::cout << e.what() << std::endl;
		return 20;
	}

	po::notify(vm);

	// �ɼ� ó��
	if (vm.count("help"))
	{
		// help ���
		cout << desc << "\n";
		return 1;
	}
	if (vm.count("port"))
	{
		config.bind_port = vm["port"].as<uint16_t>();
	}
	else
	{
		cout << "port required" << "\n";
		return 1;
	}
	if (vm.count("db-host"))
	{
		config.db_host = vm["db-host"].as<string>();
	}
	if (vm.count("db-user"))
	{
		config.db_host = vm["db-user"].as<string>();
	}
	if (vm.count("db-pass"))
	{
		config.db_host = vm["db-pass"].as<string>();
	}
	if (vm.count("db-schema"))
	{
		config.db_host = vm["db-schema"].as<string>();
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (SetServerConfig(argc, argv) != 0)
		return;

	try
	{
		auto server = std::make_shared<GameServer>();
		// ����
		server->Run();
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << e.what();
	}

    return 0;
}

