#pragma once
#include "TypeDef.h"
#include "MySQL.h"

namespace mmog
{

struct Account
{
public:
	int id;
	string acc_name;
	string password;

	static Ptr<Account> Insert(MySQLPool& db, const string& acc_name, const string& password)
	{
		std::stringstream ss;
		ss << "INSERT INTO account_tb (acc_name, password) "
			<< "VALUES ('" << acc_name << "','" << password << "')";
		
		db.Excute(ss.str());

		return Fetch(db, acc_name);
	}

	static Ptr<Account> Fetch(MySQLPool& db, const string& acc_name)
	{
		std::stringstream ss;
		ss << "SELECT id, acc_name, password FROM account_tb "
			<< "WHERE acc_name='" << acc_name << "'";
		
		auto result_set = db.Excute(ss.str());
		if (!result_set->next())
			return nullptr;

		auto account = std::make_shared<Account>();
		account->id = result_set->getInt("id");
		account->acc_name = result_set->getString("acc_name");
		account->password = result_set->getString("password");

		return account;
	}
};

enum MapType
{
	FIELD = 1,
	DUNGEON = 2,
};

struct Map
{
public:
	int id;
	string name;
	int width;
	int height;
	MapType type;
};

}

