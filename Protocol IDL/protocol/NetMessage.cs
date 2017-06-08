// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol
{

using global::System;
using global::FlatBuffers;

public struct NetMessage : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static NetMessage GetRootAsNetMessage(ByteBuffer _bb) { return GetRootAsNetMessage(_bb, new NetMessage()); }
  public static NetMessage GetRootAsNetMessage(ByteBuffer _bb, NetMessage obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public NetMessage __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public MessageT MessageType { get { int o = __p.__offset(4); return o != 0 ? (MessageT)__p.bb.Get(o + __p.bb_pos) : MessageT.NONE; } }
  public TTable? Message<TTable>() where TTable : struct, IFlatbufferObject { int o = __p.__offset(6); return o != 0 ? (TTable?)__p.__union<TTable>(o) : null; }

  public static Offset<NetMessage> CreateNetMessage(FlatBufferBuilder builder,
      MessageT message_type = MessageT.NONE,
      int messageOffset = 0) {
    builder.StartObject(2);
    NetMessage.AddMessage(builder, messageOffset);
    NetMessage.AddMessageType(builder, message_type);
    return NetMessage.EndNetMessage(builder);
  }

  public static void StartNetMessage(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddMessageType(FlatBufferBuilder builder, MessageT messageType) { builder.AddByte(0, (byte)messageType, 0); }
  public static void AddMessage(FlatBufferBuilder builder, int messageOffset) { builder.AddOffset(1, messageOffset, 0); }
  public static Offset<NetMessage> EndNetMessage(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<NetMessage>(o);
  }
  public static void FinishNetMessageBuffer(FlatBufferBuilder builder, Offset<NetMessage> offset) { builder.Finish(offset.Value); }
};


}
