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
#include <gisunnet/types.h>

using namespace boost;
using namespace gisunnet;

int main()
{
	Buffer buf(64,256);
	buf.Write('a');
	buf.Write('b');
	buf.Write('c');
	buf.Write('d');
	buf.Write('e');
	buf.Write('f');

	uint8_t array[5] = { 'g','h','i','j','k' };
	buf.InsertBytes(63, array, 0, 5);

}