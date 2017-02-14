// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GameServer.h"

#include "MySQL.h"

int main()
{
	/*auto& server = GameServer::GetInstance();
	server.Initialize();
	server.Run(8084);*/
	
	MySQLPool mysql_pool("127.0.0.1:8413", "nusigmik", "56561163", "Project_MMOG", 4);

	try
	{
		//auto conn = mysql_pool.GetConnection();
		//auto stmt = conn->createStatement();

		//auto res = stmt->executeQuery("SELECT id, user_id FROM user_tb");
		//while (res->next())
		//{
		//	//std::cout << res << "\n";
		//	std::cout << res->getInt(1);
		//	sql::SQLString user_id = res->getString(2);
		//	std::cout << user_id.c_str();
		//	std::cout << res->getInt("id");
		//}

		//delete res;
		//delete stmt;
		//mysql_pool.ReleaseConnection(conn);

		auto f = mysql_pool.ExcuteAsync("SELECT id, user_id FROM user_tb");
		auto f2 = mysql_pool.ExcuteAsync("SELECT id, user_id FROM user_tb");
		auto f3 = mysql_pool.ExcuteAsync("SELECT id, user_id FROM user_tb");
		auto f4 = mysql_pool.ExcuteAsync("SELECT id, user_id FROM user_tb");
		auto f5 = mysql_pool.ExcuteAsync("SELECT id, user_id FROM user_tb");

		auto res = f.get();
		while (res->next())
		{
			//std::cout << res << "\n";
			int id = res->getInt(1);
			sql::SQLString user_id = res->getString(2);
			std::cout << id << " " << user_id.c_str() << "\n";
		}

		auto res2 = f2.get();
		while (res2->next())
		{
			//std::cout << res << "\n";
			int id = res2->getInt(1);
			sql::SQLString user_id = res2->getString(2);
			std::cout << id << " " << user_id.c_str() << "\n";
		}

		auto res3 = f3.get();
		while (res3->next())
		{
			//std::cout << res << "\n";
			int id = res3->getInt(1);
			sql::SQLString user_id = res3->getString(2);
			std::cout << id << " " << user_id.c_str() << "\n";
		}
		/*auto driver = get_driver_instance();
		std::string url = "127.0.0.1:8413";
		std::string user = "nusigmik";
		std::string pass = "56561163";

		auto conn = driver->connect(sql::SQLString(url), sql::SQLString(user), sql::SQLString(pass));
		conn->setSchema("Project_MMOG");*/
	}
	catch (const sql::SQLException& e)
	{
		std::cerr << e.what();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
	}

    return 0;
}

