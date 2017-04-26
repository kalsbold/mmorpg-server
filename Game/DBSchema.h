#pragma once
#include "TypeDef.h"
#include "MySQL.h"

namespace mmog {
namespace db_schema {

struct Account
{
public:
	int    id;
	string acc_name;
	string password;

	static Ptr<Account> Create(MySQLPool& db, const string& acc_name, const string& password)
	{
		std::stringstream ss;
		ss << "INSERT INTO account_tb (acc_name, password) "
			<< "VALUES ('" << acc_name << "','" << password << "')";

		db.Excute(ss.str());

		return Fetch(db, acc_name);
	}

	static Ptr<Account> Fetch(MySQLPool& db, const string& acc_name)
	{
		auto conn = db.GetConnection();
		PstmtPtr pstmt(conn->prepareStatement("SELECT id, acc_name, password FROM account_tb WHERE acc_name=?"));
		pstmt->setString(1, acc_name);
		ResultSetPtr result_set(pstmt->executeQuery());

		/*std::stringstream ss;
		ss << "SELECT id, acc_name, password FROM account_tb "
			<< "WHERE acc_name='" << acc_name << "'";

		auto result_set = db.Excute(ss.str());*/

		if (!result_set->next())
			return nullptr;

		auto account = std::make_shared<Account>();
		account->id = result_set->getInt("id");
		account->acc_name = result_set->getString("acc_name");
		account->password = result_set->getString("password");

		return account;
	}
};

enum MapType : int
{
	NONE = 0,
	FIELD = 1,
	DUNGEON = 2,
};

struct Map
{
public:
	int     id;
	string  name;
	int     width;
	int     height;
	MapType type;
};

enum class ClassType : int
{
	NONE = 0,
	Knight = 1,
	Archer = 2,
	Mage = 3,
};

struct CharacterAttribute
{
public:
	ClassType class_type;
	int		  level;
	int       hp;
	int       mp;
	int       att;
	int       def;
};

struct Character
{
public:
	int       id;
	int       acc_id;
	string    name;
	ClassType class_type;
	int       exp;
	int       level;
	int       max_hp;
	int       hp;
	int       max_mp;
	int       mp;
	int       att;
	int       def;
	int       map_id;
	Vector3   pos;
	float     rotation_y;
	bool      first_play;

	static Ptr<Character> Create(MySQLPool& db, int account_id, const string& name, ClassType class_type)
	{
		std::stringstream ss;
		ss << "INSERT INTO character_tb (acc_id, name, class_type) "
			<< "VALUES (" << account_id << ",'" << name << "'," << (int)class_type << ")";

		db.Excute(ss.str());

		return Fetch(db, account_id, name);
	}

	static std::vector<Ptr<Character>> Fetch(MySQLPool& db, int account_id)
	{
		std::stringstream ss;
		ss << "SELECT * FROM character_tb "
			<< "WHERE acc_id='" << account_id << "'";

		auto result_set = db.Excute(ss.str());

		std::vector<Ptr<Character>> characters;
		while (result_set->next())
		{
			auto c = std::make_shared<Character>();
			SetCharacter(c, result_set);
			characters.push_back(c);
		}

		return characters;
	}

	static Ptr<Character> Fetch(MySQLPool& db, int account_id, const string& name)
	{
		std::stringstream ss;
		ss << "SELECT * FROM character_tb "
			<< "WHERE acc_id='" << account_id << "AND name='" << name << "'";

		auto result_set = db.Excute(ss.str());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>();
		SetCharacter(c, result_set);

		return c;
	}

	static Ptr<Character> Fetch(MySQLPool& db, int character_id, int account_id)
	{
		std::stringstream ss;
		ss << "SELECT * FROM character_tb "
			<< "WHERE id='" << character_id << "AND acc_id='" << account_id << "'";

		auto result_set = db.Excute(ss.str());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>();
		SetCharacter(c, result_set);

		return c;
	}

	static Ptr<Character> Fetch(MySQLPool& db, const string& name)
	{
		std::stringstream ss;
		ss << "SELECT * FROM character_tb "
			<< "WHERE name='" << name << "'";

		auto result_set = db.Excute(ss.str());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>();
		SetCharacter(c, result_set);

		return c;
	}

	void Update(MySQLPool& db)
	{
		std::stringstream ss;
		ss << "UPDATE character_tb SET"
			<< " exp=" << exp
			<< ",level=" << level
			<< ",max_hp=" << max_hp
			<< ",hp=" << hp
			<< ",max_mp=" << max_mp
			<< ",mp=" << mp
			<< ",att=" << att
			<< ",def=" << def
			<< ",map_id=" << map_id
			<< ",pos_x=" << pos.X
			<< ",pos_y=" << pos.Y
			<< ",pos_z=" << pos.Z
			<< ",rotation_y=" << rotation_y
			<< ",first_play=" << first_play
			<< " WHERE id=" << id;

		db.Excute(ss.str());
	}

	void Delete(MySQLPool& db)
	{
		std::stringstream ss;
		ss << "DELETE FROM character_tb WHERE id=" << id;

		db.Excute(ss.str());
	}

	void SetAttribute(CharacterAttribute& attribute)
	{
		level = attribute.level;
		max_hp = attribute.hp;
		hp = attribute.hp;
		max_mp = attribute.mp;
		mp = attribute.mp;
		att = attribute.att;
		def = attribute.def;
	}

private:
	static void SetCharacter(Ptr<Character>& c, ResultSetPtr& result_set)
	{
		c->id = result_set->getInt("id");
		c->acc_id = result_set->getInt("acc_id");
		c->name = result_set->getString("name");
		c->class_type = (ClassType)result_set->getInt("class_type");
		c->exp = result_set->getInt("exp");
		c->level = result_set->getInt("level");
		c->max_hp = result_set->getInt("max_hp");
		c->hp = result_set->getInt("hp");
		c->max_mp = result_set->getInt("max_mp");
		c->mp = result_set->getInt("mp");
		c->att = result_set->getInt("att");
		c->def = result_set->getInt("def");
		c->map_id = result_set->getInt("map_id");
		c->pos.X = (float)result_set->getDouble("pos_x");
		c->pos.Y = (float)result_set->getDouble("pos_y");
		c->pos.Z = (float)result_set->getDouble("pos_z");
		c->rotation_y = (float)result_set->getDouble("rotation_y");
		c->first_play = result_set->getBoolean("first_play");
	}
};

}
}
