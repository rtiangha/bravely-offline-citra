// Copyright 2024 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <sirit/sirit.h>

namespace Pica {
struct ShaderSetup;
}

namespace Pica::Shader {
struct VSConfig;
struct Profile;
} // namespace Pica::Shader

namespace Pica::Shader::Generator {
struct PicaVSConfig;
} // namespace Pica::Shader::Generator

namespace Pica::Shader::Generator::SPIRV {

using Sirit::Id;

struct VectorIds {
    /// Returns the type id of the vector with the provided size
    [[nodiscard]] constexpr Id Get(u32 size) const {
        return ids[size - 2];
    }

    std::array<Id, 3> ids;
};

class VertexModule : public Sirit::Module {

public:
    explicit VertexModule(const PicaVSConfig& config, const Profile& profile);
    ~VertexModule();

    /// Emits SPIR-V bytecode corresponding to the provided pica vertex configuration
    void Generate();

private:
    void DefineArithmeticTypes();
    void DefineEntryPoint();
    void DefineInterface();

private:
    const PicaVSConfig& config;
    const Profile& profile;

    Id void_id{};
    Id bool_id{};
    Id f32_id{};
    Id i32_id{};
    Id u32_id{};

    VectorIds vec_ids{};
    VectorIds ivec_ids{};
    VectorIds uvec_ids{};
    VectorIds bvec_ids{};
};

/**
 * Generates the SPIRV vertex shader program source code for the given VS program
 * @param config ShaderCacheKey object generated for the current Pica state, used for the shader
 *               configuration (NOTE: Use state in this struct only, not the Pica registers!)
 * @param separable_shader generates shader that can be used for separate shader object
 * @returns String of the shader source code; empty on failure
 */
std::vector<u32> GenerateVertexShader(const Pica::ShaderSetup& setup, const PicaVSConfig& config,
                                      const Profile& profile);
} // namespace Pica::Shader::Generator::SPIRV
