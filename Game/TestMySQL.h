#pragma once
#include "MySQL.h"

void TestMySQL()
{
	MySQLPool mysql_pool("127.0.0.1:8413", "nusigmik", "56561163", "Project_MMOG", 4);

	try
	{
		auto conn = mysql_pool.GetConnection();
		auto stmt = conn->createStatement();

		auto res = stmt->execute("INSERT INTO account_tb (acc_name, password) VALUES ('gisun06','1234')");
		auto res_set = stmt->getResultSet();
		auto count = stmt->getUpdateCount();
		//while (res->next())
		//{
		//	//std::cout << res << "\n";
		//	std::cout << res->getInt(1);
		//	sql::SQLString user_id = res->getString(2);
		//	std::cout << user_id.c_str();
		//	std::cout << res->getInt("id");
		//}

		delete res_set;
		delete stmt;

		auto stmt2 = conn->createStatement();
		auto res2 = stmt2->execute("SELECT id, acc_name FROM account_tb");
		auto res_set2 = stmt2->getResultSet();
		while (res_set2->next())
		{
			//std::cout << res << "\n";
			int id = res_set2->getInt(1);
			std::string acc_name = res_set2->getString(2).c_str();
			std::cout << id << " " << acc_name << "\n";
		}

		delete res_set2;
		delete stmt2;

		auto stmt3 = conn->createStatement();
		auto res3 = stmt3->execute("DELETE FROM account_tb WHERE acc_name = 'gisun06'");
		auto res_set3 = stmt3->getResultSet();
		auto count2 = stmt3->getUpdateCount();
		//while (res->next())
		//{
		//	//std::cout << res << "\n";
		//	std::cout << res->getInt(1);
		//	sql::SQLString user_id = res->getString(2);
		//	std::cout << user_id.c_str();
		//	std::cout << res->getInt("id");
		//}

		delete res_set3;
		delete stmt3;

		mysql_pool.ReleaseConnection(conn);

		//auto f = mysql_pool.ExcuteAsync("SELECT id, acc_name FROM account_tb");

		//auto res = f.get();
		//while (res->next())
		//{
		//	//std::cout << res << "\n";
		//	int id = res->getInt(1);
		//	std::string acc_name = res->getString(2).c_str();
		//	std::cout << id << " " << acc_name << "\n";
		//}

		//r = mysql_pool.Excute("DELETE FROM account_tb WHERE acc_name = 'gisun05';");
	}
	catch (const sql::SQLException& e)
	{
		std::cerr << e.what();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
	}
}