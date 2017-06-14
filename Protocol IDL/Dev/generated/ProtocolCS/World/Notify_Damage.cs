// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.World
{

using global::System;
using global::FlatBuffers;

public struct Notify_Damage : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static Notify_Damage GetRootAsNotify_Damage(ByteBuffer _bb) { return GetRootAsNotify_Damage(_bb, new Notify_Damage()); }
  public static Notify_Damage GetRootAsNotify_Damage(ByteBuffer _bb, Notify_Damage obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public Notify_Damage __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public DamageData? DamageList(int j) { int o = __p.__offset(4); return o != 0 ? (DamageData?)(new DamageData()).__assign(__p.__indirect(__p.__vector(o) + j * 4), __p.bb) : null; }
  public int DamageListLength { get { int o = __p.__offset(4); return o != 0 ? __p.__vector_len(o) : 0; } }

  public static Offset<Notify_Damage> CreateNotify_Damage(FlatBufferBuilder builder,
      VectorOffset damage_listOffset = default(VectorOffset)) {
    builder.StartObject(1);
    Notify_Damage.AddDamageList(builder, damage_listOffset);
    return Notify_Damage.EndNotify_Damage(builder);
  }

  public static void StartNotify_Damage(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddDamageList(FlatBufferBuilder builder, VectorOffset damageListOffset) { builder.AddOffset(0, damageListOffset.Value, 0); }
  public static VectorOffset CreateDamageListVector(FlatBufferBuilder builder, Offset<DamageData>[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static void StartDamageListVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static Offset<Notify_Damage> EndNotify_Damage(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Notify_Damage>(o);
  }
};


}