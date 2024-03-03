// Copyright 2024 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <sirit/sirit.h>

#include "common/unique_function.h"

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
    explicit VertexModule();
    ~VertexModule();

private:
    template <bool global = true>
    [[nodiscard]] Id DefineVar(Id type, spv::StorageClass storage_class) {
        const Id pointer_type_id{TypePointer(storage_class, type)};
        return global ? AddGlobalVariable(pointer_type_id, storage_class)
                      : AddLocalVariable(pointer_type_id, storage_class);
    }

    /// Defines an input variable
    [[nodiscard]] Id DefineInput(Id type, u32 location) {
        const Id input_id{DefineVar(type, spv::StorageClass::Input)};
        Decorate(input_id, spv::Decoration::Location, location);
        return input_id;
    }

    /// Defines an output variable
    [[nodiscard]] Id DefineOutput(Id type, u32 location) {
        const Id output_id{DefineVar(type, spv::StorageClass::Output)};
        Decorate(output_id, spv::Decoration::Location, location);
        return output_id;
    }

    void DefineArithmeticTypes();
    void DefineEntryPoint();
    void DefineInterface();

    Id WriteFuncSanitizeVertex();

public:
    struct EmitterIDs {
        Id void_id{};
        Id bool_id{};
        Id f32_id{};
        Id i32_id{};
        Id u32_id{};

        VectorIds vec_ids{};
        VectorIds ivec_ids{};
        VectorIds uvec_ids{};
        VectorIds bvec_ids{};

        // Input vertex attributes
        Id vert_in_position_id{};
        Id vert_in_color_id{};
        Id vert_in_texcoord0_id{};
        Id vert_in_texcoord1_id{};
        Id vert_in_texcoord2_id{};
        Id vert_in_texcoord0_w_id{};
        Id vert_in_normquat_id{};
        Id vert_in_view_id{};

        // Output vertex attributes
        Id vert_out_color_id{};
        Id vert_out_texcoord0_id{};
        Id vert_out_texcoord1_id{};
        Id vert_out_texcoord2_id{};
        Id vert_out_texcoord0_w_id{};
        Id vert_out_normquat_id{};
        Id vert_out_view_id{};

        // Uniforms

        // vs_data
        Id ptr_vs_data;
        Id ptr_enable_clip1;
        Id ptr_clip_coef;

        // Built-ins
        Id gl_position;
        Id gl_clip_distance;

        // Functions
        Id sanitize_vertex;
    } ids;

    /// Generate code using the provided SPIRV emitter context
    void Generate(Common::UniqueFunction<void, Sirit::Module&, const EmitterIDs&> proc);

    /// Emits SPIR-V bytecode corresponding to the provided pica vertex configuration
    void Generate(const PicaVSConfig& config, const Profile& profile);
};

/**
 * Generates the SPIRV vertex shader program source code that accepts vertices from software shader
 * and directly passes them to the fragment shader.
 * @returns SPIRV shader assembly; empty on failure
 */
std::vector<u32> GenerateTrivialVertexShader(bool use_clip_planes);

/**
 * Generates the SPIRV vertex shader program source code for the given VS program
 * @param config ShaderCacheKey object generated for the current Pica state, used for the shader
 *               configuration (NOTE: Use state in this struct only, not the Pica registers!)
 * @returns SPIRV shader assembly; empty on failure
 */
std::vector<u32> GenerateVertexShader(const Pica::ShaderSetup& setup, const PicaVSConfig& config,
                                      const Profile& profile);
} // namespace Pica::Shader::Generator::SPIRV
