#pragma once

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



class MySQL
{
public:
	MySQL(const std::string& url, const std::string& username, const std::string& password,
		const std::string& database, size_t connection_count,
		const std::string& connection_charset)
	{

		driver_ = get_driver_instance();
	}

private:
	void CreateConnectionPool()
	{
		for (size_t i = 0; i < connection_count_; i++)
		{
			sql::Connection* conn = driver_->connect(url_, username_, password_);
		}
	}

	sql::Driver* driver_;
	std::string url_;
	std::string username_;
	std::string password_;
	std::string database_;
	size_t connection_count_;
	std::string connection_charset_;
};