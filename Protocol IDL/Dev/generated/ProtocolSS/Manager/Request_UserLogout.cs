// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolSS.Manager
{

using global::System;
using global::FlatBuffers;

public struct Request_UserLogout : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Request_UserLogout GetRootAsRequest_UserLogout(ByteBuffer _bb) { return GetRootAsRequest_UserLogout(_bb, new Request_UserLogout()); }
  public static Request_UserLogout GetRootAsRequest_UserLogout(ByteBuffer _bb, Request_UserLogout obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Request_UserLogout __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int AccountUid { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public bool MutateAccountUid(int account_uid) { int o = __p.__offset(4); if (o != 0) { __p.bb.PutInt(o + __p.bb_pos, account_uid); return true; } else { return false; } }

  public static Offset<Request_UserLogout> CreateRequest_UserLogout(FlatBufferBuilder builder,
      int account_uid = 0) {
    builder.StartObject(1);
    Request_UserLogout.AddAccountUid(builder, account_uid);
    return Request_UserLogout.EndRequest_UserLogout(builder);
  }

  public static void StartRequest_UserLogout(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddAccountUid(FlatBufferBuilder builder, int accountUid) { builder.AddInt(0, accountUid, 0); }
  public static Offset<Request_UserLogout> EndRequest_UserLogout(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Request_UserLogout>(o);
  }
};


}