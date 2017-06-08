// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol.login
{

using global::System;
using global::FlatBuffers;

public struct Character : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Character GetRootAsCharacter(ByteBuffer _bb) { return GetRootAsCharacter(_bb, new Character()); }
  public static Character GetRootAsCharacter(ByteBuffer _bb, Character obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Character __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int Id { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public string Name { get { int o = __p.__offset(6); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetNameBytes() { return __p.__vector_as_arraysegment(6); }
  public protocol.ClassType ClassType { get { int o = __p.__offset(8); return o != 0 ? (protocol.ClassType)__p.bb.GetInt(o + __p.bb_pos) : protocol.ClassType.NONE; } }
  public int Level { get { int o = __p.__offset(10); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }

  public static Offset<Character> CreateCharacter(FlatBufferBuilder builder,
      int id = 0,
      StringOffset nameOffset = default(StringOffset),
      protocol.ClassType class_type = protocol.ClassType.NONE,
      int level = 0) {
    builder.StartObject(4);
    Character.AddLevel(builder, level);
    Character.AddClassType(builder, class_type);
    Character.AddName(builder, nameOffset);
    Character.AddId(builder, id);
    return Character.EndCharacter(builder);
  }

  public static void StartCharacter(FlatBufferBuilder builder) { builder.StartObject(4); }
  public static void AddId(FlatBufferBuilder builder, int id) { builder.AddInt(0, id, 0); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(1, nameOffset.Value, 0); }
  public static void AddClassType(FlatBufferBuilder builder, protocol.ClassType classType) { builder.AddInt(2, (int)classType, 0); }
  public static void AddLevel(FlatBufferBuilder builder, int level) { builder.AddInt(3, level, 0); }
  public static Offset<Character> EndCharacter(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Character>(o);
  }
};


}
