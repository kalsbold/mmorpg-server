// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol.world
{

using global::System;
using global::FlatBuffers;

public struct Reply_EnterWorldNextSuccess : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Reply_EnterWorldNextSuccess GetRootAsReply_EnterWorldNextSuccess(ByteBuffer _bb) { return GetRootAsReply_EnterWorldNextSuccess(_bb, new Reply_EnterWorldNextSuccess()); }
  public static Reply_EnterWorldNextSuccess GetRootAsReply_EnterWorldNextSuccess(ByteBuffer _bb, Reply_EnterWorldNextSuccess obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Reply_EnterWorldNextSuccess __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public PlayerCharacter? PlayerCharacter { get { int o = __p.__offset(4); return o != 0 ? (PlayerCharacter?)(new PlayerCharacter()).__assign(__p.__indirect(o + __p.bb_pos), __p.bb) : null; } }

  public static Offset<Reply_EnterWorldNextSuccess> CreateReply_EnterWorldNextSuccess(FlatBufferBuilder builder,
      Offset<PlayerCharacter> player_characterOffset = default(Offset<PlayerCharacter>)) {
    builder.StartObject(1);
    Reply_EnterWorldNextSuccess.AddPlayerCharacter(builder, player_characterOffset);
    return Reply_EnterWorldNextSuccess.EndReply_EnterWorldNextSuccess(builder);
  }

  public static void StartReply_EnterWorldNextSuccess(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddPlayerCharacter(FlatBufferBuilder builder, Offset<PlayerCharacter> playerCharacterOffset) { builder.AddOffset(0, playerCharacterOffset.Value, 0); }
  public static Offset<Reply_EnterWorldNextSuccess> EndReply_EnterWorldNextSuccess(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Reply_EnterWorldNextSuccess>(o);
  }
};


}