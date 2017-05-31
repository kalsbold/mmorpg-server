// automatically generated by the FlatBuffers compiler, do not modify

namespace protocol.world
{

using System;
using FlatBuffers;

public struct NotifyAttack : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static NotifyAttack GetRootAsNotifyAttack(ByteBuffer _bb) { return GetRootAsNotifyAttack(_bb, new NotifyAttack()); }
  public static NotifyAttack GetRootAsNotifyAttack(ByteBuffer _bb, NotifyAttack obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public NotifyAttack __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string Uuid { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetUuidBytes() { return __p.__vector_as_arraysegment(4); }
  public float Rotation { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }

  public static Offset<NotifyAttack> CreateNotifyAttack(FlatBufferBuilder builder,
      StringOffset uuidOffset = default(StringOffset),
      float rotation = 0.0f) {
    builder.StartObject(2);
    NotifyAttack.AddRotation(builder, rotation);
    NotifyAttack.AddUuid(builder, uuidOffset);
    return NotifyAttack.EndNotifyAttack(builder);
  }

  public static void StartNotifyAttack(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddUuid(FlatBufferBuilder builder, StringOffset uuidOffset) { builder.AddOffset(0, uuidOffset.Value, 0); }
  public static void AddRotation(FlatBufferBuilder builder, float rotation) { builder.AddFloat(1, rotation, 0.0f); }
  public static Offset<NotifyAttack> EndNotifyAttack(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<NotifyAttack>(o);
  }
};


}
