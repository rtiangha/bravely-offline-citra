#include "spv_shader_gen.h"
// Copyright 2024 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "video_core/pica/regs_rasterizer.h"
#include "video_core/shader/generator/shader_gen.h"
// #include "video_core/shader/generator/spv_shader_decompiler.h"
#include "video_core/shader/generator/spv_shader_gen.h"

using VSOutputAttributes = Pica::RasterizerRegs::VSOutputAttributes;

namespace Pica::Shader::Generator::SPIRV {

constexpr u32 SPIRV_VERSION_1_3 = 0x00010300;

VertexModule::VertexModule() : Sirit::Module{SPIRV_VERSION_1_3} {
    DefineArithmeticTypes();
    DefineInterface();

    ids.sanitize_vertex = WriteFuncSanitizeVertex();

    DefineEntryPoint();
}

VertexModule::~VertexModule() = default;

void VertexModule::DefineArithmeticTypes() {
    ids.void_id = Name(TypeVoid(), "void_id");
    ids.bool_id = Name(TypeBool(), "bool_id");
    ids.f32_id = Name(TypeFloat(32), "f32_id");
    ids.i32_id = Name(TypeSInt(32), "i32_id");
    ids.u32_id = Name(TypeUInt(32), "u32_id");

    for (u32 size = 2; size <= 4; size++) {
        const u32 i = size - 2;
        ids.bvec_ids.ids[i] = Name(TypeVector(ids.bool_id, size), fmt::format("bvec{}_id", size));
        ids.vec_ids.ids[i] = Name(TypeVector(ids.f32_id, size), fmt::format("vec{}_id", size));
        ids.ivec_ids.ids[i] = Name(TypeVector(ids.i32_id, size), fmt::format("ivec{}_id", size));
        ids.uvec_ids.ids[i] = Name(TypeVector(ids.u32_id, size), fmt::format("uvec{}_id", size));
    }
}

void VertexModule::DefineEntryPoint() {
    AddCapability(spv::Capability::Shader);
    SetMemoryModel(spv::AddressingModel::Logical, spv::MemoryModel::GLSL450);

    const Id main_type{TypeFunction(TypeVoid())};
    const Id main_func{OpFunction(TypeVoid(), spv::FunctionControlMask::MaskNone, main_type)};

    const Id interface_ids[] = {
        // Inputs
        ids.vert_in_position_id,
        ids.vert_in_color_id,
        ids.vert_in_texcoord0_id,
        ids.vert_in_texcoord1_id,
        ids.vert_in_texcoord2_id,
        ids.vert_in_texcoord0_w_id,
        ids.vert_in_normquat_id,
        ids.vert_in_view_id,
        // Outputs
        ids.gl_position,
        ids.gl_clip_distance,
        ids.vert_out_color_id,
        ids.vert_out_texcoord0_id,
        ids.vert_out_texcoord1_id,
        ids.vert_out_texcoord2_id,
        ids.vert_out_texcoord0_w_id,
        ids.vert_out_normquat_id,
        ids.vert_out_view_id,
    };

    AddEntryPoint(spv::ExecutionModel::Vertex, main_func, "main", interface_ids);
}

void VertexModule::DefineInterface() {
    // Define interface block

    /// Inputs
    ids.vert_in_position_id =
        Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_POSITION), "vert_in_position");
    ids.vert_in_color_id = Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_COLOR), "vert_in_color");
    ids.vert_in_texcoord0_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD0), "vert_in_texcoord0");
    ids.vert_in_texcoord1_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD1), "vert_in_texcoord1");
    ids.vert_in_texcoord2_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD2), "vert_in_texcoord2");
    ids.vert_in_texcoord0_w_id =
        Name(DefineInput(ids.f32_id, ATTRIBUTE_TEXCOORD0_W), "vert_in_texcoord0_w");
    ids.vert_in_normquat_id =
        Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_NORMQUAT), "vert_in_normquat");
    ids.vert_in_view_id = Name(DefineInput(ids.vec_ids.Get(3), ATTRIBUTE_VIEW), "vert_in_view");

    /// Outputs
    ids.vert_out_color_id =
        Name(DefineOutput(ids.vec_ids.Get(4), ATTRIBUTE_COLOR), "vert_out_color");
    ids.vert_out_texcoord0_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD0), "vert_out_texcoord0");
    ids.vert_out_texcoord1_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD1), "vert_out_texcoord1");
    ids.vert_out_texcoord2_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD2), "vert_out_texcoord2");
    ids.vert_out_texcoord0_w_id =
        Name(DefineOutput(ids.f32_id, ATTRIBUTE_TEXCOORD0_W), "vert_out_texcoord0_w");
    ids.vert_out_normquat_id =
        Name(DefineOutput(ids.vec_ids.Get(4), ATTRIBUTE_NORMQUAT), "vert_out_normquat");
    ids.vert_out_view_id = Name(DefineOutput(ids.vec_ids.Get(3), ATTRIBUTE_VIEW), "vert_out_view");

    /// Uniforms

    // vs_data
    const Id type_vs_data = Name(TypeStruct(ids.u32_id, ids.vec_ids.Get(4)), "vs_data");
    Decorate(type_vs_data, spv::Decoration::Block);

    ids.ptr_vs_data = AddGlobalVariable(TypePointer(spv::StorageClass::Uniform, type_vs_data),
                                        spv::StorageClass::Uniform);

    Decorate(ids.ptr_vs_data, spv::Decoration::DescriptorSet, 0);
    Decorate(ids.ptr_vs_data, spv::Decoration::Binding, 1);

    MemberName(type_vs_data, 0, "enable_clip1");
    MemberName(type_vs_data, 1, "clip_coef");

    MemberDecorate(type_vs_data, 0, spv::Decoration::Offset, 0);
    MemberDecorate(type_vs_data, 1, spv::Decoration::Offset, 16);

    /// Built-ins
    ids.gl_position = DefineVar(ids.vec_ids.Get(4), spv::StorageClass::Output);
    Decorate(ids.gl_position, spv::Decoration::BuiltIn, spv::BuiltIn::Position);

    ids.gl_clip_distance =
        DefineVar(TypeArray(ids.f32_id, Constant(ids.u32_id, 2)), spv::StorageClass::Output);
    Decorate(ids.gl_clip_distance, spv::Decoration::BuiltIn, spv::BuiltIn::ClipDistance);
}

Id VertexModule::WriteFuncSanitizeVertex() {
    const Id func_type = TypeFunction(ids.vec_ids.Get(4), ids.vec_ids.Get(4));
    const Id func =
        Name(OpFunction(ids.vec_ids.Get(4), spv::FunctionControlMask::MaskNone, func_type),
             "SanitizeVertex");
    const Id arg_pos = OpFunctionParameter(ids.vec_ids.Get(4));

    AddLabel(OpLabel());

    const Id result = AddLocalVariable(TypePointer(spv::StorageClass::Function, ids.vec_ids.Get(4)),
                                       spv::StorageClass::Function);
    OpStore(result, arg_pos);

    const Id pos_z = OpCompositeExtract(ids.f32_id, arg_pos, 2);
    const Id pos_w = OpCompositeExtract(ids.f32_id, arg_pos, 3);

    const Id ndc_z = OpFDiv(ids.f32_id, pos_z, pos_w);

    // if (ndc_z > 0.f && ndc_z < 0.000001f)
    const Id test_1 =
        OpLogicalAnd(ids.bool_id, OpFOrdGreaterThan(ids.bool_id, ndc_z, Constant(ids.f32_id, 0.0f)),
                     OpFOrdLessThan(ids.bool_id, ndc_z, Constant(ids.f32_id, 0.000001f)));

    {
        const Id true_label = OpLabel();
        const Id end_label = OpLabel();

        OpSelectionMerge(end_label, spv::SelectionControlMask::MaskNone);
        OpBranchConditional(test_1, true_label, end_label);
        AddLabel(true_label);

        // .z = 0.0f;
        OpStore(result,
                OpCompositeInsert(ids.vec_ids.Get(4), ConstantNull(ids.f32_id), arg_pos, 2));

        OpBranch(end_label);
        AddLabel(end_label);
    }

    // if (ndc_z < -1.f && ndc_z > -1.00001f)
    const Id test_2 =
        OpLogicalAnd(ids.bool_id, OpFOrdLessThan(ids.bool_id, ndc_z, Constant(ids.f32_id, -1.0f)),
                     OpFOrdGreaterThan(ids.bool_id, ndc_z, Constant(ids.f32_id, -1.00001f)));
    {
        const Id true_label = OpLabel();
        const Id end_label = OpLabel();

        OpSelectionMerge(end_label, spv::SelectionControlMask::MaskNone);
        OpBranchConditional(test_2, true_label, end_label);
        AddLabel(true_label);

        // .z = -.w;
        const Id neg_w = OpFNegate(ids.f32_id, OpCompositeExtract(ids.f32_id, arg_pos, 3));
        OpStore(result, OpCompositeInsert(ids.vec_ids.Get(4), neg_w, arg_pos, 2));

        OpBranch(end_label);
        AddLabel(end_label);
    }

    OpReturnValue(OpLoad(ids.vec_ids.Get(4), result));
    OpFunctionEnd();
    return func;
}

void VertexModule::Generate(Common::UniqueFunction<void, Sirit::Module&, const EmitterIDs&> proc) {
    AddLabel(OpLabel());

    ids.ptr_enable_clip1 = OpAccessChain(TypePointer(spv::StorageClass::Uniform, ids.u32_id),
                                         ids.ptr_vs_data, Constant(ids.u32_id, 0));

    ids.ptr_clip_coef = OpAccessChain(TypePointer(spv::StorageClass::Uniform, ids.vec_ids.Get(4)),
                                      ids.ptr_vs_data, Constant(ids.u32_id, 1));

    proc(*this, ids);
    OpReturn();
    OpFunctionEnd();
}

void VertexModule::Generate(const PicaVSConfig& config, const Profile& profile) {
    AddLabel(OpLabel());
    OpReturn();
    OpFunctionEnd();
}

std::vector<u32> GenerateTrivialVertexShader(bool use_clip_planes) {
    VertexModule module;
    module.Generate([use_clip_planes](Sirit::Module& code,
                                      const VertexModule::EmitterIDs& ids) -> void {
        const Id pos_sanitized =
            code.OpFunctionCall(ids.vec_ids.Get(4), ids.sanitize_vertex,
                                code.OpLoad(ids.vec_ids.Get(4), ids.vert_in_position_id));

        // Negate Z
        const Id neg_z =
            code.OpFNegate(ids.f32_id, code.OpCompositeExtract(ids.f32_id, pos_sanitized, 2));
        const Id negated_z = code.OpCompositeInsert(ids.vec_ids.Get(4), neg_z, pos_sanitized, 2);

        code.OpStore(ids.gl_position, negated_z);

        // Pass-through
        code.OpStore(ids.vert_out_color_id, code.OpLoad(ids.vec_ids.Get(4), ids.vert_in_color_id));
        code.OpStore(ids.vert_out_texcoord0_id,
                     code.OpLoad(ids.vec_ids.Get(2), ids.vert_in_texcoord0_id));
        code.OpStore(ids.vert_out_texcoord1_id,
                     code.OpLoad(ids.vec_ids.Get(2), ids.vert_in_texcoord1_id));
        code.OpStore(ids.vert_out_texcoord2_id,
                     code.OpLoad(ids.vec_ids.Get(2), ids.vert_in_texcoord2_id));
        code.OpStore(ids.vert_out_texcoord0_w_id,
                     code.OpLoad(ids.f32_id, ids.vert_in_texcoord0_w_id));
        code.OpStore(ids.vert_out_normquat_id,
                     code.OpLoad(ids.vec_ids.Get(4), ids.vert_in_normquat_id));
        code.OpStore(ids.vert_out_view_id, code.OpLoad(ids.vec_ids.Get(3), ids.vert_in_view_id));

        if (use_clip_planes) {
            code.OpStore(code.OpAccessChain(code.TypePointer(spv::StorageClass::Output, ids.f32_id),
                                            ids.gl_clip_distance, code.Constant(ids.u32_id, 0)),
                         neg_z);

            const Id enable_clip1 =
                code.OpINotEqual(ids.bool_id, code.OpLoad(ids.u32_id, ids.ptr_enable_clip1),
                                 code.Constant(ids.u32_id, 0));

            {
                const Id true_label = code.OpLabel();
                const Id false_label = code.OpLabel();
                const Id end_label = code.OpLabel();

                code.OpSelectionMerge(end_label, spv::SelectionControlMask::MaskNone);
                code.OpBranchConditional(enable_clip1, true_label, false_label);
                {
                    code.AddLabel(true_label);

                    code.OpStore(
                        code.OpAccessChain(code.TypePointer(spv::StorageClass::Output, ids.f32_id),
                                           ids.gl_clip_distance, code.Constant(ids.u32_id, 1)),
                        code.OpDot(ids.f32_id, code.OpLoad(ids.vec_ids.Get(4), ids.ptr_clip_coef),
                                   pos_sanitized));

                    code.OpBranch(end_label);
                }
                {
                    code.AddLabel(false_label);

                    code.OpStore(
                        code.OpAccessChain(code.TypePointer(spv::StorageClass::Output, ids.f32_id),
                                           ids.gl_clip_distance, code.Constant(ids.u32_id, 1)),
                        code.ConstantNull(ids.f32_id));

                    code.OpBranch(end_label);
                }
                code.AddLabel(end_label);
            }
        }
    });
    return module.Assemble();
}

std::vector<u32> GenerateVertexShader(const ShaderSetup& setup, const PicaVSConfig& config,
                                      const Profile& profile) {
    VertexModule module;
    module.Generate(config, profile);
    return module.Assemble();
}

} // namespace Pica::Shader::Generator::SPIRV