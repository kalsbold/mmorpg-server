// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol.world
{

using global::System;
using global::FlatBuffers;

public struct Request_EnterWorldNext : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Request_EnterWorldNext GetRootAsRequest_EnterWorldNext(ByteBuffer _bb) { return GetRootAsRequest_EnterWorldNext(_bb, new Request_EnterWorldNext()); }
  public static Request_EnterWorldNext GetRootAsRequest_EnterWorldNext(ByteBuffer _bb, Request_EnterWorldNext obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Request_EnterWorldNext __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }


  public static void StartRequest_EnterWorldNext(FlatBufferBuilder builder) { builder.StartObject(0); }
  public static Offset<Request_EnterWorldNext> EndRequest_EnterWorldNext(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_EnterWorldNext>(o);
  }
};


}
