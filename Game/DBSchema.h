#pragma once
#include "Common.h"
#include "TypeDef.h"
#include "MySQL.h"
#include "protocol_cs_generated.h"

namespace fb = flatbuffers;
namespace PCS = ProtocolCS;

namespace db_schema {

class Account
{
public:
	int         uid;
	std::string user_name;
	std::string password;

	static Ptr<Account> Create(Ptr<MySQLPool> db, const std::string& user_name, const std::string& password)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"INSERT INTO account_tb(user_name, password) VALUES(?,?)"));
		pstmt->setString(1, user_name.c_str());
		pstmt->setString(2, password.c_str());

		if (pstmt->executeUpdate() == 0)
			return nullptr;

		return Fetch(db, user_name);
	}

	static Ptr<Account> Fetch(Ptr<MySQLPool> db, const std::string& user_name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement("SELECT uid, user_name, password FROM account_tb WHERE user_name=?"));
		pstmt->setString(1, user_name.c_str());
		
		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto account = std::make_shared<Account>();
		account->uid = result_set->getInt("uid");
		account->user_name = result_set->getString("user_name").c_str();
		account->password = result_set->getString("password").c_str();
		return account;
	}

	static Ptr<Account> Fetch(Ptr<MySQLPool> db, int uid)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement("SELECT uid, user_name, password FROM account_tb WHERE uid=?"));
		pstmt->setInt(1, uid);

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto account = std::make_shared<Account>();
		account->uid = result_set->getInt("uid");
		account->user_name = result_set->getString("user_name").c_str();
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

class MapGate
{
public:
    int uid;
    int map_id;
    Vector3 pos;
    int dest_uid;
};

class HeroAttribute
{
public:
	ClassType class_type;
	int		  level;
	int       hp;
	int       mp;
	int       att;
	int       def;
};

class Hero
{
public:
	int            uid;
	int            account_uid;
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
	float          rotation;

	static Ptr<Hero> Create(Ptr<MySQLPool> db, int account_uid, const std::string& name, ClassType class_type)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"INSERT INTO  hero_tb (account_uid, name, class_type) VALUES(?,?,?)"));
		pstmt->setInt(1, account_uid);
		pstmt->setString(2, name.c_str());
		pstmt->setInt(3, (int)class_type);

		if (pstmt->executeUpdate() == 0)
			return nullptr;

		return Fetch(db, account_uid, name);
	}

	static std::vector<Ptr<Hero>> Fetch(Ptr<MySQLPool> db, int account_uid)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM hero_tb WHERE account_uid=?"));
		pstmt->setInt(1, account_uid);

		ResultSetPtr result_set(pstmt->executeQuery());
		std::vector<Ptr<Hero>> hero_vec;
		while (result_set->next())
		{
			auto c = std::make_shared<Hero>();
			Set(c, result_set);
			hero_vec.push_back(std::move(c));
		}

		return hero_vec;
	}

	static Ptr<Hero> Fetch(Ptr<MySQLPool> db, int account_uid, const std::string& name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM hero_tb WHERE account_uid=? AND name=?"));
		pstmt->setInt(1, account_uid);
		pstmt->setString(2, name.c_str());

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Hero>();
		Set(c, result_set);
		return c;
	}

	static Ptr<Hero> Fetch(Ptr<MySQLPool> db, int uid, int account_uid)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM hero_tb WHERE uid=? AND account_uid=?"));
		pstmt->setInt(1, uid);
		pstmt->setInt(2, account_uid);

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Hero>();
		Set(c, result_set);
		return c;
	}

	static Ptr<Hero> Fetch(Ptr<MySQLPool> db, const std::string& name)
	{
		ConnectionPtr conn = db->GetConnection();
		PstmtPtr pstmt(conn->prepareStatement(
			"SELECT * FROM hero_tb WHERE name=?"));
		pstmt->setString(1, name.c_str());

		ResultSetPtr result_set(pstmt->executeQuery());
		if (!result_set->next())
			return nullptr;

		auto c = std::make_shared<Hero>();
		Set(c, result_set);
		return c;
	}

	bool Update(Ptr<MySQLPool> db)
	{
		if (!db) return false;

		ConnectionPtr conn = db->GetConnection();
		StmtPtr stmt(conn->createStatement());
		
		std::stringstream ss;
		ss << "UPDATE hero_tb SET"
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
			<< ",rotation=" << rotation
			<< " WHERE uid=" << uid;

		return stmt->execute(ss.str().c_str());
	}

	bool Delete(Ptr<MySQLPool> db)
	{
		if (!db) return false;

		ConnectionPtr conn = db->GetConnection();
		StmtPtr stmt(conn->createStatement());

		std::stringstream ss;
		ss << "DELETE FROM hero_tb WHERE uid=" << uid;

		return stmt->execute(ss.str().c_str());
	}

	void SetAttribute(const HeroAttribute& attribute)
	{
		level = attribute.level;
		max_hp = attribute.hp;
		hp = attribute.hp;
		max_mp = attribute.mp;
		mp = attribute.mp;
		att = attribute.att;
		def = attribute.def;
	}

    fb::Offset<PCS::World::Hero> Serialize(fb::FlatBufferBuilder & fbb) const
    {
        //ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
        return PCS::World::CreateHeroDirect(fbb,
            nullptr,
            uid,
            name.c_str(),
            (PCS::ClassType)class_type,
            exp,
            level,
            max_hp,
            hp,
            max_mp,
            mp,
            att,
            def,
            map_id,
            //&pos,
            &PCS::Vec3(pos.X, pos.Y, pos.Z),
            rotation
        );
    }

private:
	static void Set(Ptr<Hero>& c, ResultSetPtr& result_set)
	{
		c->uid = result_set->getInt("uid");
		c->account_uid = result_set->getInt("account_uid");
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
		c->rotation = (float)result_set->getDouble("rotation");
	}
};

class Monster
{
public:
    int            uid;
    int            type_id;
    std::string    name;
    int            level;
    int            max_hp;
    int            max_mp;
    int            att;
    int            def;
};

class MonsterSpawn
{
public:
    int      uid;
    int      map_id;
    int      monster_uid;
    Vector3  pos;
    duration interval_s;
};


class Skill
{
public:
    int           skill_id;
    ClassType     class_type;
    TargetingType targeting_type;
    float         range;
    float         radius;
    float         angle;
    int           cost;
    float         cast_time;
    float         cool_down;
    int           damage;
};

class HeroSpawn
{
public:
    int      uid;
    int      map_id;
    Vector3  pos;
};

}