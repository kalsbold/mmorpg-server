// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol.world
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

  public MoveInfo? Move { get { int o = __p.__offset(4); return o != 0 ? (MoveInfo?)(new MoveInfo()).__assign(o + __p.bb_pos, __p.bb) : null; } }

  public static void StartRequest_Move(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddMove(FlatBufferBuilder builder, Offset<MoveInfo> moveOffset) { builder.AddStruct(0, moveOffset.Value, 0); }
  public static Offset<Request_Move> EndRequest_Move(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_Move>(o);
  }
};


}