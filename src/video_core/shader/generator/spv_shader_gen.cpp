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
        ids.vert_in_position_id,   ids.vert_in_color_id,        ids.vert_in_texcoord0_id,
        ids.vert_in_texcoord1_id,  ids.vert_in_texcoord2_id,    ids.vert_in_texcoord0_w_id,
        ids.vert_in_normquat_id,   ids.vert_in_view_id,         ids.gl_position,
        ids.vert_out_color_id,     ids.vert_out_texcoord0_id,   ids.vert_out_texcoord1_id,
        ids.vert_out_texcoord2_id, ids.vert_out_texcoord0_w_id, ids.vert_out_normquat_id,
        ids.vert_out_view_id,
    };

    AddEntryPoint(spv::ExecutionModel::Vertex, main_func, "main", interface_ids);
}

void VertexModule::DefineInterface() {
    // Define interface block

    // Inputs
    ids.vert_in_position_id =
        Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_POSITION), "vert_in_position_id");
    ids.vert_in_color_id =
        Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_COLOR), "vert_in_color_id");
    ids.vert_in_texcoord0_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD0), "vert_in_texcoord0_id");
    ids.vert_in_texcoord1_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD1), "vert_in_texcoord1_id");
    ids.vert_in_texcoord2_id =
        Name(DefineInput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD2), "vert_in_texcoord2_id");
    ids.vert_in_texcoord0_w_id =
        Name(DefineInput(ids.f32_id, ATTRIBUTE_TEXCOORD0_W), "vert_in_texcoord0_w_id");
    ids.vert_in_normquat_id =
        Name(DefineInput(ids.vec_ids.Get(4), ATTRIBUTE_NORMQUAT), "vert_in_normquat_id");
    ids.vert_in_view_id = Name(DefineInput(ids.vec_ids.Get(3), ATTRIBUTE_VIEW), "vert_in_view_id");

    // Outputs
    ids.vert_out_color_id =
        Name(DefineOutput(ids.vec_ids.Get(4), ATTRIBUTE_COLOR), "vert_out_color_id");
    ids.vert_out_texcoord0_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD0), "vert_out_texcoord0_id");
    ids.vert_out_texcoord1_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD1), "vert_out_texcoord1_id");
    ids.vert_out_texcoord2_id =
        Name(DefineOutput(ids.vec_ids.Get(2), ATTRIBUTE_TEXCOORD2), "vert_out_texcoord2_id");
    ids.vert_out_texcoord0_w_id =
        Name(DefineOutput(ids.f32_id, ATTRIBUTE_TEXCOORD0_W), "vert_out_texcoord0_w_id");
    ids.vert_out_normquat_id =
        Name(DefineOutput(ids.vec_ids.Get(4), ATTRIBUTE_NORMQUAT), "vert_out_normquat_id");
    ids.vert_out_view_id =
        Name(DefineOutput(ids.vec_ids.Get(3), ATTRIBUTE_VIEW), "vert_out_view_id");

    // Built-ins
    ids.gl_position = DefineVar(ids.vec_ids.Get(4), spv::StorageClass::Output);
    Decorate(ids.gl_position, spv::Decoration::BuiltIn, spv::BuiltIn::Position);
}

void VertexModule::Generate(Common::UniqueFunction<void, Sirit::Module&, const EmitterIDs&> proc) {
    AddLabel(OpLabel());
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
    module.Generate([](Sirit::Module& code, const VertexModule::EmitterIDs& ids) -> void {
        code.OpStore(ids.gl_position, code.OpLoad(ids.vec_ids.Get(4), ids.vert_in_position_id));

        // Negate Z
        const Id pos_z = code.OpAccessChain(code.TypePointer(spv::StorageClass::Output, ids.f32_id),
                                            ids.gl_position, code.Constant(ids.u32_id, 2));

        code.OpStore(pos_z, code.OpFNegate(ids.f32_id, code.OpLoad(ids.f32_id, pos_z)));

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