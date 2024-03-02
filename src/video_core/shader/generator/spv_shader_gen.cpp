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

VertexModule::VertexModule(const PicaVSConfig& config_, const Profile& profile_)
    : Sirit::Module{SPIRV_VERSION_1_3}, config{config_}, profile{profile_} {
    DefineArithmeticTypes();
    DefineInterface();
    DefineEntryPoint();
}

VertexModule::~VertexModule() = default;

void VertexModule::Generate() {
    AddLabel(OpLabel());
    OpReturn();
    OpFunctionEnd();
}

void VertexModule::DefineArithmeticTypes() {
    void_id = Name(TypeVoid(), "void_id");
    bool_id = Name(TypeBool(), "bool_id");
    f32_id = Name(TypeFloat(32), "f32_id");
    i32_id = Name(TypeSInt(32), "i32_id");
    u32_id = Name(TypeUInt(32), "u32_id");

    for (u32 size = 2; size <= 4; size++) {
        const u32 i = size - 2;
        vec_ids.ids[i] = Name(TypeVector(f32_id, size), fmt::format("vec{}_id", size));
        ivec_ids.ids[i] = Name(TypeVector(i32_id, size), fmt::format("ivec{}_id", size));
        uvec_ids.ids[i] = Name(TypeVector(u32_id, size), fmt::format("uvec{}_id", size));
        bvec_ids.ids[i] = Name(TypeVector(bool_id, size), fmt::format("bvec{}_id", size));
    }
}

void VertexModule::DefineEntryPoint() {
    AddCapability(spv::Capability::Shader);
    SetMemoryModel(spv::AddressingModel::Logical, spv::MemoryModel::GLSL450);

    const Id main_type{TypeFunction(TypeVoid())};
    const Id main_func{OpFunction(TypeVoid(), spv::FunctionControlMask::MaskNone, main_type)};

    AddEntryPoint(spv::ExecutionModel::Vertex, main_func, "main");
}

void VertexModule::DefineInterface() {
    // Define interface block
}

std::vector<u32> GenerateVertexShader(const ShaderSetup& setup, const PicaVSConfig& config,
                                      const Profile& profile) {
    VertexModule module(config, profile);
    module.Generate();
    return module.Assemble();
}

} // namespace Pica::Shader::Generator::SPIRV