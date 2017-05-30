// automatically generated by the FlatBuffers compiler, do not modify

namespace protocol.world
{

using System;
using FlatBuffers;

public struct ActionMove : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static ActionMove GetRootAsActionMove(ByteBuffer _bb) { return GetRootAsActionMove(_bb, new ActionMove()); }
  public static ActionMove GetRootAsActionMove(ByteBuffer _bb, ActionMove obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public ActionMove __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public float Rotation { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public Vec3? Pos { get { int o = __p.__offset(6); return o != 0 ? (Vec3?)(new Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }
  public Vec3? Velocity { get { int o = __p.__offset(8); return o != 0 ? (Vec3?)(new Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }

  public static void StartActionMove(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddRotation(FlatBufferBuilder builder, float rotation) { builder.AddFloat(0, rotation, 0.0f); }
  public static void AddPos(FlatBufferBuilder builder, Offset<Vec3> posOffset) { builder.AddStruct(1, posOffset.Value, 0); }
  public static void AddVelocity(FlatBufferBuilder builder, Offset<Vec3> velocityOffset) { builder.AddStruct(2, velocityOffset.Value, 0); }
  public static Offset<ActionMove> EndActionMove(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<ActionMove>(o);
  }
};


}
