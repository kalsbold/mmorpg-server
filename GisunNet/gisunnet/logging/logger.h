#pragma once

#include "gisunnet/logging/services/basic_logger.hpp"
#include "gisunnet/logging/services/logger_service.hpp"

namespace gisunnet {
namespace logging {

	enum class log_level
	{
		trace = 0,
		debug,
		info,
		warning,
		error,
		fatal,
	};

	typedef services::basic_logger<services::logger_service> logger;

	logger& get()
	{
		static boost::asio::io_service io_service;
		static services::basic_logger<services::logger_service> logger(io_service, "");

		return logger;
	}

	void set_log_level(log_level level)
	{

	}

	/*void log(logger& logger, )
	{

	}*/

	template<typename T >
	logger& operator<< (logger& logger, T const& value)
	{
		logger.log(value);
		return logger;
	}
}
	

}

/*
#define LOG_CRITICAL   \
        if (crow::logger::get_current_log_level() <= crow::LogLevel::Critical) \
            crow::logger("CRITICAL", crow::LogLevel::Critical)
#define LOG_ERROR      \
        if (crow::logger::get_current_log_level() <= crow::LogLevel::Error) \
            crow::logger("ERROR   ", crow::LogLevel::Error)
#define LOG_WARNING    \
        if (crow::logger::get_current_log_level() <= crow::LogLevel::Warning) \
            crow::logger("WARNING ", crow::LogLevel::Warning)
#define LOG_INFO       \
        if (crow::logger::get_current_log_level() <= crow::LogLevel::Info) \
            crow::logger("INFO    ", crow::LogLevel::Info)
#define LOG_DEBUG      \
        if (crow::logger::get_current_log_level() <= crow::LogLevel::Debug) \
            crow::logger("DEBUG   ", crow::LogLevel::Debug)
*/