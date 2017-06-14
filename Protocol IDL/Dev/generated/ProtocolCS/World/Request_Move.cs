// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.World
{

using global::System;
using global::FlatBuffers;

public struct Request_Move : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Request_Move GetRootAsRequest_Move(ByteBuffer _bb) { return GetRootAsRequest_Move(_bb, new Request_Move()); }
  public static Request_Move GetRootAsRequest_Move(ByteBuffer _bb, Request_Move obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Request_Move __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public ProtocolCS.Vec3? Position { get { int o = __p.__offset(4); return o != 0 ? (ProtocolCS.Vec3?)(new ProtocolCS.Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }
  public float Rotation { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public bool MutateRotation(float rotation) { int o = __p.__offset(6); if (o != 0) { __p.bb.PutFloat(o + __p.bb_pos, rotation); return true; } else { return false; } }
  public ProtocolCS.Vec3? Velocity { get { int o = __p.__offset(8); return o != 0 ? (ProtocolCS.Vec3?)(new ProtocolCS.Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }

  public static void StartRequest_Move(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddPosition(FlatBufferBuilder builder, Offset<ProtocolCS.Vec3> positionOffset) { builder.AddStruct(0, positionOffset.Value, 0); }
  public static void AddRotation(FlatBufferBuilder builder, float rotation) { builder.AddFloat(1, rotation, 0.0f); }
  public static void AddVelocity(FlatBufferBuilder builder, Offset<ProtocolCS.Vec3> velocityOffset) { builder.AddStruct(2, velocityOffset.Value, 0); }
  public static Offset<Request_Move> EndRequest_Move(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_Move>(o);
  }
};


}