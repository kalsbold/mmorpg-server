// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.Login
{

using global::System;
using global::FlatBuffers;

public struct Request_CreateHero : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Request_CreateHero GetRootAsRequest_CreateHero(ByteBuffer _bb) { return GetRootAsRequest_CreateHero(_bb, new Request_CreateHero()); }
  public static Request_CreateHero GetRootAsRequest_CreateHero(ByteBuffer _bb, Request_CreateHero obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Request_CreateHero __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string Name { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetNameBytes() { return __p.__vector_as_arraysegment(4); }
  public ProtocolCS.ClassType ClassType { get { int o = __p.__offset(6); return o != 0 ? (ProtocolCS.ClassType)__p.bb.GetInt(o + __p.bb_pos) : ProtocolCS.ClassType.NONE; } }
  public bool MutateClassType(ProtocolCS.ClassType class_type) { int o = __p.__offset(6); if (o != 0) { __p.bb.PutInt(o + __p.bb_pos, (int)class_type); return true; } else { return false; } }

  public static Offset<Request_CreateHero> CreateRequest_CreateHero(FlatBufferBuilder builder,
      StringOffset nameOffset = default(StringOffset),
      ProtocolCS.ClassType class_type = ProtocolCS.ClassType.NONE) {
    builder.StartObject(2);
    Request_CreateHero.AddClassType(builder, class_type);
    Request_CreateHero.AddName(builder, nameOffset);
    return Request_CreateHero.EndRequest_CreateHero(builder);
  }

  public static void StartRequest_CreateHero(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(0, nameOffset.Value, 0); }
  public static void AddClassType(FlatBufferBuilder builder, ProtocolCS.ClassType classType) { builder.AddInt(1, (int)classType, 0); }
  public static Offset<Request_CreateHero> EndRequest_CreateHero(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_CreateHero>(o);
  }
};


}
