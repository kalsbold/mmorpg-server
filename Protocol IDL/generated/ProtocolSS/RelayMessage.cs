// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolSS
{

using global::System;
using global::FlatBuffers;

public struct RelayMessage : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static RelayMessage GetRootAsRelayMessage(ByteBuffer _bb) { return GetRootAsRelayMessage(_bb, new RelayMessage()); }
  public static RelayMessage GetRootAsRelayMessage(ByteBuffer _bb, RelayMessage obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public RelayMessage __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int SourceId { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public bool MutateSourceId(int source_id) { int o = __p.__offset(4); if (o != 0) { __p.bb.PutInt(o + __p.bb_pos, source_id); return true; } else { return false; } }
  public int DestinationsId(int j) { int o = __p.__offset(6); return o != 0 ? __p.bb.GetInt(__p.__vector(o) + j * 4) : (int)0; }
  public int DestinationsIdLength { get { int o = __p.__offset(6); return o != 0 ? __p.__vector_len(o) : 0; } }
  public ArraySegment<byte>? GetDestinationsIdBytes() { return __p.__vector_as_arraysegment(6); }
  public bool MutateDestinationsId(int j, int destinations_id) { int o = __p.__offset(6); if (o != 0) { __p.bb.PutInt(__p.__vector(o) + j * 4, destinations_id); return true; } else { return false; } }
  public MessageType MessageType { get { int o = __p.__offset(8); return o != 0 ? (MessageType)__p.bb.Get(o + __p.bb_pos) : MessageType.NONE; } }
  public bool MutateMessageType(MessageType message_type) { int o = __p.__offset(8); if (o != 0) { __p.bb.Put(o + __p.bb_pos, (byte)message_type); return true; } else { return false; } }
  public TTable? Message<TTable>() where TTable : struct, IFlatbufferObject { int o = __p.__offset(10); return o != 0 ? (TTable?)__p.__union<TTable>(o) : null; }

  public static Offset<RelayMessage> CreateRelayMessage(FlatBufferBuilder builder,
      int source_id = 0,
      VectorOffset destinations_idOffset = default(VectorOffset),
      MessageType message_type = MessageType.NONE,
      int messageOffset = 0) {
    builder.StartObject(4);
    RelayMessage.AddMessage(builder, messageOffset);
    RelayMessage.AddDestinationsId(builder, destinations_idOffset);
    RelayMessage.AddSourceId(builder, source_id);
    RelayMessage.AddMessageType(builder, message_type);
    return RelayMessage.EndRelayMessage(builder);
  }

  public static void StartRelayMessage(FlatBufferBuilder builder) { builder.StartObject(4); }
  public static void AddSourceId(FlatBufferBuilder builder, int sourceId) { builder.AddInt(0, sourceId, 0); }
  public static void AddDestinationsId(FlatBufferBuilder builder, VectorOffset destinationsIdOffset) { builder.AddOffset(1, destinationsIdOffset.Value, 0); }
  public static VectorOffset CreateDestinationsIdVector(FlatBufferBuilder builder, int[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddInt(data[i]); return builder.EndVector(); }
  public static void StartDestinationsIdVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddMessageType(FlatBufferBuilder builder, MessageType messageType) { builder.AddByte(2, (byte)messageType, 0); }
  public static void AddMessage(FlatBufferBuilder builder, int messageOffset) { builder.AddOffset(3, messageOffset, 0); }
  public static Offset<RelayMessage> EndRelayMessage(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<RelayMessage>(o);
  }
};


}
