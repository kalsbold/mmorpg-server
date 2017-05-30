// automatically generated by the FlatBuffers compiler, do not modify

namespace protocol.login
{

using System;
using FlatBuffers;

public struct RequestLogin : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static RequestLogin GetRootAsRequestLogin(ByteBuffer _bb) { return GetRootAsRequestLogin(_bb, new RequestLogin()); }
  public static RequestLogin GetRootAsRequestLogin(ByteBuffer _bb, RequestLogin obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public RequestLogin __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string AccName { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetAccNameBytes() { return __p.__vector_as_arraysegment(4); }
  public string Password { get { int o = __p.__offset(6); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetPasswordBytes() { return __p.__vector_as_arraysegment(6); }

  public static Offset<RequestLogin> CreateRequestLogin(FlatBufferBuilder builder,
      StringOffset acc_nameOffset = default(StringOffset),
      StringOffset passwordOffset = default(StringOffset)) {
    builder.StartObject(2);
    RequestLogin.AddPassword(builder, passwordOffset);
    RequestLogin.AddAccName(builder, acc_nameOffset);
    return RequestLogin.EndRequestLogin(builder);
  }

  public static void StartRequestLogin(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddAccName(FlatBufferBuilder builder, StringOffset accNameOffset) { builder.AddOffset(0, accNameOffset.Value, 0); }
  public static void AddPassword(FlatBufferBuilder builder, StringOffset passwordOffset) { builder.AddOffset(1, passwordOffset.Value, 0); }
  public static Offset<RequestLogin> EndRequestLogin(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<RequestLogin>(o);
  }
};


}
