#pragma once
#include "Common.h"
#include "MySQL.h"

namespace db_entity {

class EntityBase
{
public:
	EntityBase()
		: db_(nullptr)
	{}
	explicit EntityBase(Ptr<MySQLPool> db)
		: db_(db)
	{}
	virtual ~EntityBase() {};

	void SetDB(Ptr<MySQLPool> db)
	{
		db_ = db;
	}
	Ptr<MySQLPool> GetDB()
	{
		return db_;
	}

	virtual bool Update() { return true; }
	virtual bool Delete() { return true; }

private:
	Ptr<MySQLPool> db_;
};

class Account : public EntityBase
{
public:
	using EntityBase::EntityBase;

	int    id;
	std::string acc_name;
	std::string password;

	static Ptr<Account> Create(Ptr<MySQLPool> db, const std::string& acc_name, const std::string& password)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"INSERT INTO account_tb(acc_name, password) VALUES(?,?)"));
		pstmt->setString(1, acc_name.c_str());
		pstmt->setString(2, password.c_str());

		if (pstmt->executeUpdate() == 0)
			return nullptr;

		return Fetch(db, acc_name);
	}

	static Ptr<Account> Fetch(Ptr<MySQLPool> db, const std::string& acc_name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement("SELECT id, acc_name, password FROM account_tb WHERE acc_name=?"));
		pstmt->setString(1, acc_name.c_str());
		
		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto account = std::make_shared<Account>(db);
		account->id = result_set->getInt("id");
		account->acc_name = result_set->getString("acc_name").c_str();
		account->password = result_set->getString("password").c_str();
		return account;
	}
};

class Map
{
public:
	int         id;
	std::string name;
	int         width;
	int         height;
	MapType	    type;
};

class CharacterAttribute
{
public:
	ClassType class_type;
	int		  level;
	int       hp;
	int       mp;
	int       att;
	int       def;
};

class Character : public EntityBase
{
public:
	using EntityBase::EntityBase;

	int            id;
	int            acc_id;
	std::string    name;
	ClassType      class_type;
	int            exp;
	int            level;
	int            max_hp;
	int            hp;
	int            max_mp;
	int            mp;
	int            att;
	int            def;
	int            map_id;
	Vector3        pos;
	float          rotation_y;
	bool           first_play;

	static Ptr<Character> Create(Ptr<MySQLPool> db, int account_id, const std::string& name, ClassType class_type)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"INSERT INTO  character_tb (acc_id, name, class_type) VALUES(?,?,?)"));
		pstmt->setInt(1, account_id);
		pstmt->setString(2, name.c_str());
		pstmt->setInt(3, (int)class_type);

		if (pstmt->executeUpdate() == 0)
			return nullptr;

		return Fetch(db, account_id, name);
	}

	static std::vector<Ptr<Character>> Fetch(Ptr<MySQLPool> db, int account_id)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM character_tb WHERE acc_id=?"));
		pstmt->setInt(1, account_id);

		ResultSetPtr result_set(pstmt->executeQuery());
		std::vector<Ptr<Character>> characters;
		while (result_set->next())
		{
			auto c = std::make_shared<Character>(db);
			Set(c, result_set);
			characters.push_back(std::move(c));
		}

		return characters;
	}

	static Ptr<Character> Fetch(Ptr<MySQLPool> db, int account_id, const std::string& name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM character_tb WHERE acc_id=? AND name=?"));
		pstmt->setInt(1, account_id);
		pstmt->setString(2, name.c_str());

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>(db);
		Set(c, result_set);
		return c;
	}

	static Ptr<Character> Fetch(Ptr<MySQLPool> db, int character_id, int account_id)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM character_tb WHERE id=? AND acc_id=?"));
		pstmt->setInt(1, character_id);
		pstmt->setInt(2, account_id);

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>(db);
		Set(c, result_set);
		return c;
	}

	static Ptr<Character> Fetch(Ptr<MySQLPool> db, const std::string& name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM character_tb WHERE name=?"));
		pstmt->setString(1, name.c_str());

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Character>(db);
		Set(c, result_set);
		return c;
	}

	bool Update() override
	{
		auto db = GetDB();
		if (!db) return false;

		ConnectionPtr conn = db->GetConnection();
		StmtPtr stmt(conn->createStatement());
		
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

		return stmt->execute(ss.str().c_str());
	}

	bool Delete() override
	{
		auto db = GetDB();
		if (!db) return false;

		ConnectionPtr conn = db->GetConnection();
		StmtPtr stmt(conn->createStatement());

		std::stringstream ss;
		ss << "DELETE FROM character_tb WHERE id=" << id;

		return stmt->execute(ss.str().c_str());
	}

	void SetAttribute(const CharacterAttribute& attribute)
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
	static void Set(Ptr<Character>& c, ResultSetPtr& result_set)
	{
		c->id = result_set->getInt("id");
		c->acc_id = result_set->getInt("acc_id");
		c->name = result_set->getString("name").c_str();
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