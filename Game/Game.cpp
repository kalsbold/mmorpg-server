// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <gisunnet/network/Server.h>
#include <gisunnet/network/IoServicePool.h>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)

#define LOG(message)\
	BOOST_LOG(my_logger::get()) << message

void InitLog()
{
	//logging::add_file_log
	//(
	//	keywords::file_name = "sample_%N.log",
	//	keywords::rotation_size = 10 * 1024 * 1024, //10mb마다 rotate
	//	keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), //12시마다 rotate
	//	keywords::format = "[%TimeStamp%]: %Message%"
	//);
	
	logging::add_common_attributes();
}

void Run()
{
	namespace asio = boost::asio;
	using namespace gisunnet;

	LOG("Run Game Server");

	LOG("Create IoServicePool");
	auto ios_pool = std::make_shared<IoServicePool>(4);
	
	Configuration config;
	config.io_service_pool = ios_pool;
	config.max_session_count = 1000;
	config.max_receive_buffer_size = 4 * 1024;
	config.min_receive_size = 256;
	config.no_delay = true;

	Ptr<Server> server = Server::Create(config);
	server->RegisterSessionOpenedHandler([](const Ptr<Session>& session) {
		LOG("Session Open");
	});
	server->RegisterSessionClosedHandler([](const Ptr<Session>& session, const CloseReason& reason) {
		LOG("Session Close");
	});
	server->RegisterMessageHandler([](const auto& session, const uint8_t* buf, size_t bytes) {
		LOG("Session Message");
	});

	uint16_t bind_port = 8088;

	LOG("Start Server");
	server->Start(bind_port);

	// 종료 signal
	/*asio::signal_set signals(ios_pool->PickIoService(), SIGINT, SIGTERM);
	signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
		if (!error)
		{
			LOG("Stop Server");
			server->Stop();
			LOG("Stop IoServicePool");
			ios_pool->Stop();
		}
	});*/

	while (auto ch = getchar())
	{
		if (ch == 'q')
		{
			LOG("Stop Server");
			server->Stop();
			LOG("Stop IoServicePool");
			ios_pool->Stop();
			break;
		}
	}

	// IoServicePool 이 정지될때 까지 기다린다.
	ios_pool->Wait();

	// 종료 작업.
	LOG("Stop Game Server");
}

int main()
{
	InitLog();
	Run();

    return 0;
}

