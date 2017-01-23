// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_
#define FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_

#include "flatbuffers/flatbuffers.h"

namespace MyGame {
namespace Sample {

struct Vec3;

struct Monster;

struct Weapon;

enum Color {
  Color_Red = 0,
  Color_Green = 1,
  Color_Blue = 2,
  Color_MIN = Color_Red,
  Color_MAX = Color_Blue
};

inline const char **EnumNamesColor() {
  static const char *names[] = {
    "Red",
    "Green",
    "Blue",
    nullptr
  };
  return names;
}

inline const char *EnumNameColor(Color e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesColor()[index];
}

enum Equipment {
  Equipment_NONE = 0,
  Equipment_Weapon = 1,
  Equipment_MIN = Equipment_NONE,
  Equipment_MAX = Equipment_Weapon
};

inline const char **EnumNamesEquipment() {
  static const char *names[] = {
    "NONE",
    "Weapon",
    nullptr
  };
  return names;
}

inline const char *EnumNameEquipment(Equipment e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesEquipment()[index];
}

template<typename T> struct EquipmentTraits {
  static const Equipment enum_value = Equipment_NONE;
};

template<> struct EquipmentTraits<Weapon> {
  static const Equipment enum_value = Equipment_Weapon;
};

bool VerifyEquipment(flatbuffers::Verifier &verifier, const void *obj, Equipment type);

MANUALLY_ALIGNED_STRUCT(4) Vec3 FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  Vec3() {
    memset(this, 0, sizeof(Vec3));
  }
  Vec3(const Vec3 &_o) {
    memcpy(this, &_o, sizeof(Vec3));
  }
  Vec3(float _x, float _y, float _z)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        z_(flatbuffers::EndianScalar(_z)) {
  }
  float x() const {
    return flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return flatbuffers::EndianScalar(z_);
  }
};
STRUCT_END(Vec3, 12);

struct Monster FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_POS = 4,
    VT_MANA = 6,
    VT_HP = 8,
    VT_NAME = 10,
    VT_INVENTORY = 14,
    VT_COLOR = 16,
    VT_WEAPONS = 18,
    VT_EQUIPPED_TYPE = 20,
    VT_EQUIPPED = 22
  };
  const Vec3 *pos() const {
    return GetStruct<const Vec3 *>(VT_POS);
  }
  int16_t mana() const {
    return GetField<int16_t>(VT_MANA, 150);
  }
  int16_t hp() const {
    return GetField<int16_t>(VT_HP, 100);
  }
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::Vector<uint8_t> *inventory() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_INVENTORY);
  }
  Color color() const {
    return static_cast<Color>(GetField<int8_t>(VT_COLOR, 2));
  }
  const flatbuffers::Vector<flatbuffers::Offset<Weapon>> *weapons() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Weapon>> *>(VT_WEAPONS);
  }
  Equipment equipped_type() const {
    return static_cast<Equipment>(GetField<uint8_t>(VT_EQUIPPED_TYPE, 0));
  }
  const void *equipped() const {
    return GetPointer<const void *>(VT_EQUIPPED);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<Vec3>(verifier, VT_POS) &&
           VerifyField<int16_t>(verifier, VT_MANA) &&
           VerifyField<int16_t>(verifier, VT_HP) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_INVENTORY) &&
           verifier.Verify(inventory()) &&
           VerifyField<int8_t>(verifier, VT_COLOR) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_WEAPONS) &&
           verifier.Verify(weapons()) &&
           verifier.VerifyVectorOfTables(weapons()) &&
           VerifyField<uint8_t>(verifier, VT_EQUIPPED_TYPE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_EQUIPPED) &&
           VerifyEquipment(verifier, equipped(), equipped_type()) &&
           verifier.EndTable();
  }
};

struct MonsterBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_pos(const Vec3 *pos) {
    fbb_.AddStruct(Monster::VT_POS, pos);
  }
  void add_mana(int16_t mana) {
    fbb_.AddElement<int16_t>(Monster::VT_MANA, mana, 150);
  }
  void add_hp(int16_t hp) {
    fbb_.AddElement<int16_t>(Monster::VT_HP, hp, 100);
  }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Monster::VT_NAME, name);
  }
  void add_inventory(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory) {
    fbb_.AddOffset(Monster::VT_INVENTORY, inventory);
  }
  void add_color(Color color) {
    fbb_.AddElement<int8_t>(Monster::VT_COLOR, static_cast<int8_t>(color), 2);
  }
  void add_weapons(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Weapon>>> weapons) {
    fbb_.AddOffset(Monster::VT_WEAPONS, weapons);
  }
  void add_equipped_type(Equipment equipped_type) {
    fbb_.AddElement<uint8_t>(Monster::VT_EQUIPPED_TYPE, static_cast<uint8_t>(equipped_type), 0);
  }
  void add_equipped(flatbuffers::Offset<void> equipped) {
    fbb_.AddOffset(Monster::VT_EQUIPPED, equipped);
  }
  MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  MonsterBuilder &operator=(const MonsterBuilder &);
  flatbuffers::Offset<Monster> Finish() {
    const auto end = fbb_.EndTable(start_, 10);
    auto o = flatbuffers::Offset<Monster>(end);
    return o;
  }
};

inline flatbuffers::Offset<Monster> CreateMonster(
    flatbuffers::FlatBufferBuilder &_fbb,
    const Vec3 *pos = 0,
    int16_t mana = 150,
    int16_t hp = 100,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory = 0,
    Color color = Color_Blue,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Weapon>>> weapons = 0,
    Equipment equipped_type = Equipment_NONE,
    flatbuffers::Offset<void> equipped = 0) {
  MonsterBuilder builder_(_fbb);
  builder_.add_equipped(equipped);
  builder_.add_weapons(weapons);
  builder_.add_inventory(inventory);
  builder_.add_name(name);
  builder_.add_pos(pos);
  builder_.add_hp(hp);
  builder_.add_mana(mana);
  builder_.add_equipped_type(equipped_type);
  builder_.add_color(color);
  return builder_.Finish();
}

inline flatbuffers::Offset<Monster> CreateMonsterDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const Vec3 *pos = 0,
    int16_t mana = 150,
    int16_t hp = 100,
    const char *name = nullptr,
    const std::vector<uint8_t> *inventory = nullptr,
    Color color = Color_Blue,
    const std::vector<flatbuffers::Offset<Weapon>> *weapons = nullptr,
    Equipment equipped_type = Equipment_NONE,
    flatbuffers::Offset<void> equipped = 0) {
  return CreateMonster(
      _fbb,
      pos,
      mana,
      hp,
      name ? _fbb.CreateString(name) : 0,
      inventory ? _fbb.CreateVector<uint8_t>(*inventory) : 0,
      color,
      weapons ? _fbb.CreateVector<flatbuffers::Offset<Weapon>>(*weapons) : 0,
      equipped_type,
      equipped);
}

struct Weapon FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_DAMAGE = 6
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  int16_t damage() const {
    return GetField<int16_t>(VT_DAMAGE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<int16_t>(verifier, VT_DAMAGE) &&
           verifier.EndTable();
  }
};

struct WeaponBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Weapon::VT_NAME, name);
  }
  void add_damage(int16_t damage) {
    fbb_.AddElement<int16_t>(Weapon::VT_DAMAGE, damage, 0);
  }
  WeaponBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  WeaponBuilder &operator=(const WeaponBuilder &);
  flatbuffers::Offset<Weapon> Finish() {
    const auto end = fbb_.EndTable(start_, 2);
    auto o = flatbuffers::Offset<Weapon>(end);
    return o;
  }
};

inline flatbuffers::Offset<Weapon> CreateWeapon(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    int16_t damage = 0) {
  WeaponBuilder builder_(_fbb);
  builder_.add_name(name);
  builder_.add_damage(damage);
  return builder_.Finish();
}

inline flatbuffers::Offset<Weapon> CreateWeaponDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    int16_t damage = 0) {
  return CreateWeapon(
      _fbb,
      name ? _fbb.CreateString(name) : 0,
      damage);
}

inline bool VerifyEquipment(flatbuffers::Verifier &verifier, const void *obj, Equipment type) {
  switch (type) {
    case Equipment_NONE: {
      return true;
    }
    case Equipment_Weapon: {
      auto ptr = reinterpret_cast<const Weapon *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return false;
  }
}

inline const MyGame::Sample::Monster *GetMonster(const void *buf) {
  return flatbuffers::GetRoot<MyGame::Sample::Monster>(buf);
}

inline bool VerifyMonsterBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<MyGame::Sample::Monster>(nullptr);
}

inline void FinishMonsterBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<MyGame::Sample::Monster> root) {
  fbb.Finish(root);
}

}  // namespace Sample
}  // namespace MyGame

#endif  // FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_
