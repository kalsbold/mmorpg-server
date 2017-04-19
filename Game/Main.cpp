// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost\program_options.hpp>
#include "ServerConfig.h"
#include "GameServer.h"



using namespace mmog;

void RunServer()
{
	// 게임서버 설정
	ServerConfig& config = ServerConfig::GetInstance();
	config.bind_port = 8088;
	config.thread_count = 4;
	config.db_host = "127.0.0.1:3306";
	config.db_user = "nusigmik";
	config.db_password = "56561163";
	config.db_schema = "Project_MMOG";
	config.db_connection_pool = 4;

	try
	{
		auto server = std::make_shared<GameServer>();
		server->Run();
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << e.what();
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;
	namespace po = boost::program_options;

	string server_mode;

	// 옵션 설명 정의
	po::options_description desc("Configuration");
	desc.add_options()
		("help", "produce help message")
		("server-mode", po::value<string>(&server_mode), "login, world")
		("address", po::value<string>()->default_value("0.0.0.0"))
		("port", po::value<uint16_t>())
		("thread", po::value<size_t>())
		("max_session", po::value<size_t>())
		("db-host", po::value<string>()->default_value("127.0.0.1:3306"))
		("db-user", po::value<string>())
		("db-pass", po::value<string>())
		("db-schema", po::value<string>())
		("db-conn-pool", po::value<size_t>()->default_value(1))
		;

	// 옵션이 없을 경우
	if (2 > argc)
	{
	    std::cout << desc << "\n";
	    return 1;
	}

	// 명령행 옵션 분석
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch (po::unknown_option & e)
	{
	    // 예외 1: 없는 옵션을 사용한 경우
	    std::cout << e.what() << std::endl;
	    return 10;
	}
	catch (po::invalid_option_value & e)
	{
	    // 예외 2: 옵션 값의 오류가 발생한 경우
	    std::cout << e.what() << std::endl;
	    return 11;
	}
	catch (po::invalid_command_line_syntax & e)
	{
	    // 예외 3: 옵션 값이 없을 경우
	    std::cout << e.what() << std::endl;
	    return 12;
	}
	catch (std::exception & e)
	{
	    // 예외 4: 이외의 예외 발생
	    std::cout << e.what() << std::endl;
	    return 20;
	}
	
	po::notify(vm);

	// 옵션 처리
	if (vm.count("help"))
	{
		cout << desc << "\n";
		return 1;
	}
	if (vm.count("server-mode"))
	{
		auto server_mode = vm["server-mode"].as<string>();
	}
	else
	{
		cout << "Must be required server-mode options\n";
		return 1;
	}

	if (vm.count("port"))
	{
		cout << vm["port"].as<uint16_t>();
	}

	if (vm.count("thread"))
	{
		cout << vm["thread"].as<size_t>();
	}

	//RunServer();

    return 0;
}

