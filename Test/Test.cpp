// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <mysql_connection.h>
//#include <mysql_driver.h>
//#include <mysql_error.h>
//#include <cppconn/driver.h>
//#include <cppconn/exception.h>
//#include <cppconn/resultset.h>
//#include <cppconn/statement.h>
//#include <cppconn/prepared_statement.h>
//
//int main()
//{
//	sql::Driver *driver;
//	sql::Connection *con;
//	sql::Statement *stmt;
//	sql::ResultSet *res;
//	sql::PreparedStatement *pstmt;
//
//	try {
//		driver = sql::mysql::get_mysql_driver_instance();
//		con = driver->connect("tcp://127.0.0.1:3306", sql::SQLString("root"), sql::SQLString(""));
//		con->setSchema("mob_management");
//		
//		stmt = con->createStatement();
//		stmt->execute("insert into character_table (name) values('orc'),('five')");
//		delete stmt;
//		
//		pstmt = con->prepareStatement("select * from character_table");
//		res = pstmt->executeQuery();
//		while (res->next())
//		        std::cout << res->getInt("id") << "  " << res->getString("name") << std::endl;
//		delete res;
//		delete pstmt;
//		
//		pstmt = con->prepareStatement("delete from character_table where id=?");
//		pstmt->setInt(1, 10);
//		pstmt->executeUpdate();
//		//pstmt->setInt(1, 11);
//		//pstmt->executeUpdate();
//		
//		delete pstmt;
//		
//		delete con;
//	}
//	catch (sql::SQLException &e) {
//		std::cout << e.what();
//	}
//
//	return 0;
//}

#include <iostream>
#include <future>
#include <thread>
#include <boost/asio.hpp>

using namespace boost;

std::future<bool> async_func(asio::io_service& ios)
{
	auto p = std::make_shared<std::promise<bool>>();
	auto f = p->get_future();
	ios.post([p]() {
		p->set_value(false);
	});

	return std::move(f);
}

template <class Service, class CompletionHandler>
auto post_future(Service& s, CompletionHandler&& handler)
{
	using return_type = decltype(handler());
	auto promise = std::make_shared<std::promise<return_type>>();
	auto future = promise->get_future();
	s.post([promise, handler] {
		promise->set_value(handler());
	});

	return future;
}

template <class Service, class CompletionHandler>
auto dispatch_future(Service& s, CompletionHandler&& handler)
{
	using return_type = decltype(handler());
	auto promise = std::make_shared<std::promise<return_type>>();
	auto future = promise->get_future();
	s.dispatch([promise, handler] {
		promise->set_value(handler());
	});

	return future;
}

int main()
{
	std::cout << std::this_thread::get_id() << "\n";
	asio::io_service ios;
	asio::io_service::strand strand(ios);
	//auto f = async_func(ios);
	
	auto func1 = [] {
		std::cout << std::this_thread::get_id() << " Call func1\n";
		return false;
	};
	auto func2 = [] {
		std::cout << std::this_thread::get_id() << " Call func1\n";
		return false;
	};

	auto f = post_future(strand, func1);
	auto f2 = dispatch_future(strand, func2);

	std::async(std::launch::async, [&] { ios.run(); });

	std::cout<< f.get();
	std::cout << f2.get();
}