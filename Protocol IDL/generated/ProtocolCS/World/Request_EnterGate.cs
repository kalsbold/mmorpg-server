// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.World
{

using global::System;
using global::FlatBuffers;

public struct Request_EnterGate : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Request_EnterGate GetRootAsRequest_EnterGate(ByteBuffer _bb) { return GetRootAsRequest_EnterGate(_bb, new Request_EnterGate()); }
  public static Request_EnterGate GetRootAsRequest_EnterGate(ByteBuffer _bb, Request_EnterGate obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Request_EnterGate __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int GateUid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public bool MutateGateUid(int gate_uid) { int o = __p.__offset(4); if (o != 0) { __p.bb.PutInt(o + __p.bb_pos, gate_uid); return true; } else { return false; } }

  public static Offset<Request_EnterGate> CreateRequest_EnterGate(FlatBufferBuilder builder,
      int gate_uid = 0) {
    builder.StartObject(1);
    Request_EnterGate.AddGateUid(builder, gate_uid);
    return Request_EnterGate.EndRequest_EnterGate(builder);
  }

  public static void StartRequest_EnterGate(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddGateUid(FlatBufferBuilder builder, int gateUid) { builder.AddInt(0, gateUid, 0); }
  public static Offset<Request_EnterGate> EndRequest_EnterGate(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_EnterGate>(o);
  }
};


}
