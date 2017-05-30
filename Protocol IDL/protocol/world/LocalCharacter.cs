// automatically generated by the FlatBuffers compiler, do not modify

namespace protocol.world
{

using System;
using FlatBuffers;

public struct LocalCharacter : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static LocalCharacter GetRootAsLocalCharacter(ByteBuffer _bb) { return GetRootAsLocalCharacter(_bb, new LocalCharacter()); }
  public static LocalCharacter GetRootAsLocalCharacter(ByteBuffer _bb, LocalCharacter obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public LocalCharacter __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int Uuid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public string Name { get { int o = __p.__offset(6); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetNameBytes() { return __p.__vector_as_arraysegment(6); }
  public protocol.ClassType ClassType { get { int o = __p.__offset(8); return o != 0 ? (protocol.ClassType)__p.bb.GetInt(o + __p.bb_pos) : protocol.ClassType.NONE; } }
  public int Exp { get { int o = __p.__offset(10); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int Level { get { int o = __p.__offset(12); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int MaxHp { get { int o = __p.__offset(14); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int Hp { get { int o = __p.__offset(16); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int MaxMp { get { int o = __p.__offset(18); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int Mp { get { int o = __p.__offset(20); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int Att { get { int o = __p.__offset(22); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int Def { get { int o = __p.__offset(24); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int MapId { get { int o = __p.__offset(26); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public Vec3? Pos { get { int o = __p.__offset(28); return o != 0 ? (Vec3?)(new Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }
  public float RotationY { get { int o = __p.__offset(30); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public float Speed { get { int o = __p.__offset(32); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }

  public static void StartLocalCharacter(FlatBufferBuilder builder) { builder.StartObject(15); }
  public static void AddUuid(FlatBufferBuilder builder, int uuid) { builder.AddInt(0, uuid, 0); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(1, nameOffset.Value, 0); }
  public static void AddClassType(FlatBufferBuilder builder, protocol.ClassType classType) { builder.AddInt(2, (int)classType, 0); }
  public static void AddExp(FlatBufferBuilder builder, int exp) { builder.AddInt(3, exp, 0); }
  public static void AddLevel(FlatBufferBuilder builder, int level) { builder.AddInt(4, level, 0); }
  public static void AddMaxHp(FlatBufferBuilder builder, int maxHp) { builder.AddInt(5, maxHp, 0); }
  public static void AddHp(FlatBufferBuilder builder, int hp) { builder.AddInt(6, hp, 0); }
  public static void AddMaxMp(FlatBufferBuilder builder, int maxMp) { builder.AddInt(7, maxMp, 0); }
  public static void AddMp(FlatBufferBuilder builder, int mp) { builder.AddInt(8, mp, 0); }
  public static void AddAtt(FlatBufferBuilder builder, int att) { builder.AddInt(9, att, 0); }
  public static void AddDef(FlatBufferBuilder builder, int def) { builder.AddInt(10, def, 0); }
  public static void AddMapId(FlatBufferBuilder builder, int mapId) { builder.AddInt(11, mapId, 0); }
  public static void AddPos(FlatBufferBuilder builder, Offset<Vec3> posOffset) { builder.AddStruct(12, posOffset.Value, 0); }
  public static void AddRotationY(FlatBufferBuilder builder, float rotationY) { builder.AddFloat(13, rotationY, 0.0f); }
  public static void AddSpeed(FlatBufferBuilder builder, float speed) { builder.AddFloat(14, speed, 0.0f); }
  public static Offset<LocalCharacter> EndLocalCharacter(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<LocalCharacter>(o);
  }
};


}
