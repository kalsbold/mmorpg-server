// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace protocol.world
{

using global::System;
using global::FlatBuffers;

public struct Notify_Hit : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Notify_Hit GetRootAsNotify_Hit(ByteBuffer _bb) { return GetRootAsNotify_Hit(_bb, new Notify_Hit()); }
  public static Notify_Hit GetRootAsNotify_Hit(ByteBuffer _bb, Notify_Hit obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Notify_Hit __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public HittingInfo? HitList(int j) { int o = __p.__offset(4); return o != 0 ? (HittingInfo?)(new HittingInfo()).__assign(__p.__indirect(__p.__vector(o) + j * 4), __p.bb) : null; }
  public int HitListLength { get { int o = __p.__offset(4); return o != 0 ? __p.__vector_len(o) : 0; } }

  public static Offset<Notify_Hit> CreateNotify_Hit(FlatBufferBuilder builder,
      VectorOffset hit_listOffset = default(VectorOffset)) {
    builder.StartObject(1);
    Notify_Hit.AddHitList(builder, hit_listOffset);
    return Notify_Hit.EndNotify_Hit(builder);
  }

  public static void StartNotify_Hit(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddHitList(FlatBufferBuilder builder, VectorOffset hitListOffset) { builder.AddOffset(0, hitListOffset.Value, 0); }
  public static VectorOffset CreateHitListVector(FlatBufferBuilder builder, Offset<HittingInfo>[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static void StartHitListVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static Offset<Notify_Hit> EndNotify_Hit(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Notify_Hit>(o);
  }
};


}