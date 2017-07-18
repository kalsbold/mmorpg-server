// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace ProtocolCS.World
{

using global::System;
using global::FlatBuffers;

public struct SkillActionInfo : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static SkillActionInfo GetRootAsSkillActionInfo(ByteBuffer _bb) { return GetRootAsSkillActionInfo(_bb, new SkillActionInfo()); }
  public static SkillActionInfo GetRootAsSkillActionInfo(ByteBuffer _bb, SkillActionInfo obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public SkillActionInfo __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string EntityId { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
  public ArraySegment<byte>? GetEntityIdBytes() { return __p.__vector_as_arraysegment(4); }
  public int SkillId { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public bool MutateSkillId(int skill_id) { int o = __p.__offset(6); if (o != 0) { __p.bb.PutInt(o + __p.bb_pos, skill_id); return true; } else { return false; } }
  public string Targets(int j) { int o = __p.__offset(8); return o != 0 ? __p.__string(__p.__vector(o) + j * 4) : null; }
  public int TargetsLength { get { int o = __p.__offset(8); return o != 0 ? __p.__vector_len(o) : 0; } }
  public float Rotation { get { int o = __p.__offset(10); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public bool MutateRotation(float rotation) { int o = __p.__offset(10); if (o != 0) { __p.bb.PutFloat(o + __p.bb_pos, rotation); return true; } else { return false; } }

  public static Offset<SkillActionInfo> CreateSkillActionInfo(FlatBufferBuilder builder,
      StringOffset entity_idOffset = default(StringOffset),
      int skill_id = 0,
      VectorOffset targetsOffset = default(VectorOffset),
      float rotation = 0.0f) {
    builder.StartObject(4);
    SkillActionInfo.AddRotation(builder, rotation);
    SkillActionInfo.AddTargets(builder, targetsOffset);
    SkillActionInfo.AddSkillId(builder, skill_id);
    SkillActionInfo.AddEntityId(builder, entity_idOffset);
    return SkillActionInfo.EndSkillActionInfo(builder);
  }

  public static void StartSkillActionInfo(FlatBufferBuilder builder) { builder.StartObject(4); }
  public static void AddEntityId(FlatBufferBuilder builder, StringOffset entityIdOffset) { builder.AddOffset(0, entityIdOffset.Value, 0); }
  public static void AddSkillId(FlatBufferBuilder builder, int skillId) { builder.AddInt(1, skillId, 0); }
  public static void AddTargets(FlatBufferBuilder builder, VectorOffset targetsOffset) { builder.AddOffset(2, targetsOffset.Value, 0); }
  public static VectorOffset CreateTargetsVector(FlatBufferBuilder builder, StringOffset[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static void StartTargetsVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddRotation(FlatBufferBuilder builder, float rotation) { builder.AddFloat(3, rotation, 0.0f); }
  public static Offset<SkillActionInfo> EndSkillActionInfo(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<SkillActionInfo>(o);
  }
};


}
