// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.World
{

using global::System;
using global::FlatBuffers;

public struct MoveActionInfo : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static MoveActionInfo GetRootAsMoveActionInfo(ByteBuffer _bb) { return GetRootAsMoveActionInfo(_bb, new MoveActionInfo()); }
  public static MoveActionInfo GetRootAsMoveActionInfo(ByteBuffer _bb, MoveActionInfo obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public MoveActionInfo __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string EntityId { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetEntityIdBytes() { return __p.__vector_as_arraysegment(4); }
  public ProtocolCS.Vec3? Position { get { int o = __p.__offset(6); return o != 0 ? (ProtocolCS.Vec3?)(new ProtocolCS.Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }
  public float Rotation { get { int o = __p.__offset(8); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public bool MutateRotation(float rotation) { int o = __p.__offset(8); if (o != 0) { __p.bb.PutFloat(o + __p.bb_pos, rotation); return true; } else { return false; } }
  public ProtocolCS.Vec3? Velocity { get { int o = __p.__offset(10); return o != 0 ? (ProtocolCS.Vec3?)(new ProtocolCS.Vec3()).__assign(o + __p.bb_pos, __p.bb) : null; } }

  public static void StartMoveActionInfo(FlatBufferBuilder builder) { builder.StartObject(4); }
  public static void AddEntityId(FlatBufferBuilder builder, StringOffset entityIdOffset) { builder.AddOffset(0, entityIdOffset.Value, 0); }
  public static void AddPosition(FlatBufferBuilder builder, Offset<ProtocolCS.Vec3> positionOffset) { builder.AddStruct(1, positionOffset.Value, 0); }
  public static void AddRotation(FlatBufferBuilder builder, float rotation) { builder.AddFloat(2, rotation, 0.0f); }
  public static void AddVelocity(FlatBufferBuilder builder, Offset<ProtocolCS.Vec3> velocityOffset) { builder.AddStruct(3, velocityOffset.Value, 0); }
  public static Offset<MoveActionInfo> EndMoveActionInfo(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<MoveActionInfo>(o);
  }
};


}
