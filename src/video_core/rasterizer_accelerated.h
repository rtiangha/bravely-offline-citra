// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <vector>
#include "common/vector_math.h"
#include "video_core/rasterizer_interface.h"
#include "video_core/shader/generator/pica_fs_config.h"
#include "video_core/shader/generator/shader_uniforms.h"

// SIMD includes
#if defined(__aarch64__) && defined(__ARM_NEON)
#include <arm_neon.h>
#elif defined(__x86_64__)
#include <immintrin.h>
#endif

namespace Memory {
class MemorySystem;
}

namespace Pica {
class PicaCore;
}

namespace VideoCore {

using Pica::f24;

class RasterizerAccelerated : public RasterizerInterface {
public:
    explicit RasterizerAccelerated(Memory::MemorySystem& memory, Pica::PicaCore& pica);
    virtual ~RasterizerAccelerated() = default;

    void AddTriangle(const Pica::OutputVertex& v0, const Pica::OutputVertex& v1,
                     const Pica::OutputVertex& v2) override;

    void NotifyPicaRegisterChanged(u32 id) override;

    void SyncEntireState() override;

protected:
    /// Sync fixed-function pipeline state
    virtual void SyncFixedState() = 0;

    /// Notifies that a fixed function PICA register changed to the video backend
    virtual void NotifyFixedFunctionPicaRegisterChanged(u32 id) = 0;

    /// SIMD optimized color conversion
    [[maybe_unused]] static inline Common::Vec4f ColorRGBA8(const u32 color) {
#if defined(__aarch64__) && defined(__ARM_NEON)
        const uint32x4_t rgba = {color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF,
                                 (color >> 24) & 0xFF};
        const float32x4_t converted = vcvtq_f32_u32(rgba);
        const float32x4_t result = vdivq_f32(converted, vdupq_n_f32(255.0f));
        return Common::Vec4f{vgetq_lane_f32(result, 0), vgetq_lane_f32(result, 1),
                             vgetq_lane_f32(result, 2), vgetq_lane_f32(result, 3)};
#elif defined(__x86_64__) && defined(__SSE3__)
        const __m128i rgba = _mm_set_epi32(color >> 24, color >> 16, color >> 8, color);
        const __m128i mask = _mm_set1_epi32(0xFF);
        const __m128i masked = _mm_and_si128(rgba, mask);
        const __m128 converted = _mm_cvtepi32_ps(masked);
        const __m128 result = _mm_div_ps(converted, _mm_set1_ps(255.0f));
        float temp[4];
        _mm_store_ps(temp, result);
        return Common::Vec4f{temp[0], temp[1], temp[2], temp[3]};
#else
        return Common::Vec4f{static_cast<float>(color & 0xFF) / 255.0f,
                             static_cast<float>((color >> 8) & 0xFF) / 255.0f,
                             static_cast<float>((color >> 16) & 0xFF) / 255.0f,
                             static_cast<float>((color >> 24) & 0xFF) / 255.0f};
#endif
    }

    /// SIMD optimized light color conversion
    [[maybe_unused]] static inline Common::Vec3f LightColor(
        const Pica::LightingRegs::LightColor& color) {
#if defined(__aarch64__) && defined(__ARM_NEON)
        const uint32x4_t rgb = {color.r, color.g, color.b, 0};
        const float32x4_t converted = vcvtq_f32_u32(rgb);
        const float32x4_t result = vdivq_f32(converted, vdupq_n_f32(255.0f));
        return Common::Vec3f{vgetq_lane_f32(result, 0), vgetq_lane_f32(result, 1),
                             vgetq_lane_f32(result, 2)};
#elif defined(__x86_64__) && defined(__SSE3__)
        const __m128i rgb = _mm_set_epi32(0, color.b, color.g, color.r);
        const __m128 converted = _mm_cvtepi32_ps(rgb);
        const __m128 result = _mm_div_ps(converted, _mm_set1_ps(255.0f));
        float temp[4];
        _mm_store_ps(temp, result);
        return Common::Vec3f{temp[0], temp[1], temp[2]};
#else
        return Common::Vec3u{color.r, color.g, color.b} / 255.0f;
#endif
    }

    /**
     * This is a helper function to resolve an issue when interpolating opposite quaternions. See
     * below for a detailed description of this issue (yuriks):
     *
     * For any rotation, there are two quaternions Q, and -Q, that represent the same rotation. If
     * you interpolate two quaternions that are opposite, instead of going from one rotation to
     * another using the shortest path, you'll go around the longest path. You can test if two
     * quaternions are opposite by checking if Dot(Q1, Q2) < 0. In that case, you can flip either of
     * them, therefore making Dot(Q1, -Q2) positive.
     *
     * This solution corrects this issue per-vertex before passing the quaternions to OpenGL. This
     * is correct for most cases but can still rotate around the long way sometimes. An
     * implementation which did `lerp(lerp(Q1, Q2), Q3)` (with proper weighting), applying the dot
     * product check between each step would work for those cases at the cost of being more complex
     * to implement.
     *
     * Fortunately however, the 3DS hardware happens to also use this exact same logic to work
     * around these issues, making this basic implementation actually more accurate to the hardware.
     */
    /// SIMD optimized quaternion comparison
    [[maybe_unused]] static inline bool AreQuaternionsOpposite(Common::Vec4<f24> qa,
                                                               Common::Vec4<f24> qb) {
#if defined(__aarch64__) && defined(__ARM_NEON)
        const float32x4_t a = {qa.x.ToFloat32(), qa.y.ToFloat32(), qa.z.ToFloat32(),
                               qa.w.ToFloat32()};
        const float32x4_t b = {qb.x.ToFloat32(), qb.y.ToFloat32(), qb.z.ToFloat32(),
                               qb.w.ToFloat32()};
        const float32x4_t prod = vmulq_f32(a, b);
        const float32x2_t sum = vadd_f32(vget_low_f32(prod), vget_high_f32(prod));
        const float32x2_t dot = vpadd_f32(sum, sum);
        return vget_lane_f32(dot, 0) < 0.0f;
#elif defined(__x86_64__) && defined(__SSE3__)
        const __m128 a =
            _mm_set_ps(qa.w.ToFloat32(), qa.z.ToFloat32(), qa.y.ToFloat32(), qa.x.ToFloat32());
        const __m128 b =
            _mm_set_ps(qb.w.ToFloat32(), qb.z.ToFloat32(), qb.y.ToFloat32(), qb.x.ToFloat32());
        const __m128 prod = _mm_mul_ps(a, b);
        const __m128 sum = _mm_hadd_ps(prod, prod);
        const __m128 dot = _mm_hadd_ps(sum, sum);
        return _mm_cvtss_f32(dot) < 0.0f;
#else
        Common::Vec4f a{qa.x.ToFloat32(), qa.y.ToFloat32(), qa.z.ToFloat32(), qa.w.ToFloat32()};
        Common::Vec4f b{qb.x.ToFloat32(), qb.y.ToFloat32(), qb.z.ToFloat32(), qb.w.ToFloat32()};
        return (Common::Dot(a, b) < 0.0f);
#endif
    }

    // Sync functions
    /// Syncs the depth scale to match the PICA register
    void SyncDepthScale();

    /// Syncs the depth offset to match the PICA register
    void SyncDepthOffset();

    /// Syncs the fog states to match the PICA register
    void SyncFogColor();

    /// Sync the procedural texture noise configuration to match the PICA register
    void SyncProcTexNoise();

    /// Sync the procedural texture bias configuration to match the PICA register
    void SyncProcTexBias();

    /// Syncs the alpha test states to match the PICA register
    void SyncAlphaTest();

    /// Syncs the TEV combiner color buffer to match the PICA register
    void SyncCombinerColor();

    /// Syncs the TEV constant color to match the PICA register
    void SyncTevConstColor(std::size_t tev_index,
                           const Pica::TexturingRegs::TevStageConfig& tev_stage);

    /// Syncs the lighting global ambient color to match the PICA register
    void SyncGlobalAmbient();

    /// Syncs the specified light's specular 0 color to match the PICA register
    void SyncLightSpecular0(int light_index);

    /// Syncs the specified light's specular 1 color to match the PICA register
    void SyncLightSpecular1(int light_index);

    /// Syncs the specified light's diffuse color to match the PICA register
    void SyncLightDiffuse(int light_index);

    /// Syncs the specified light's ambient color to match the PICA register
    void SyncLightAmbient(int light_index);

    /// Syncs the specified light's position to match the PICA register
    void SyncLightPosition(int light_index);

    /// Syncs the specified spot light direcition to match the PICA register
    void SyncLightSpotDirection(int light_index);

    /// Syncs the specified light's distance attenuation bias to match the PICA register
    void SyncLightDistanceAttenuationBias(int light_index);

    /// Syncs the specified light's distance attenuation scale to match the PICA register
    void SyncLightDistanceAttenuationScale(int light_index);

    /// Syncs the shadow rendering bias to match the PICA register
    void SyncShadowBias();

    /// Syncs the shadow texture bias to match the PICA register
    void SyncShadowTextureBias();

    /// Syncs the texture LOD bias to match the PICA register
    void SyncTextureLodBias(int tex_index);

    /// Syncs the texture border color to match the PICA registers
    void SyncTextureBorderColor(int tex_index);

    /// Syncs the clip plane state to match the PICA register
    void SyncClipPlane();

protected:
    /// Structure that keeps tracks of the vertex shader uniform state
    struct alignas(16) VSUniformBlockData {
        Pica::Shader::Generator::VSUniformData data{};
        bool dirty = true;
    };

    /// Structure that keeps tracks of the fragment shader uniform state
    struct alignas(16) FSUniformBlockData {
        Pica::Shader::Generator::FSUniformData data{};
        std::array<bool, Pica::LightingRegs::NumLightingSampler> lighting_lut_dirty{};
        bool lighting_lut_dirty_any = true;
        bool fog_lut_dirty = true;
        bool proctex_noise_lut_dirty = true;
        bool proctex_color_map_dirty = true;
        bool proctex_alpha_map_dirty = true;
        bool proctex_lut_dirty = true;
        bool proctex_diff_lut_dirty = true;
        bool dirty = true;
    };

    /// Structure that the hardware rendered vertices are composed of
    struct alignas(16) HardwareVertex {
        HardwareVertex() = default;
        HardwareVertex(const Pica::OutputVertex& v, bool flip_quaternion);

        Common::Vec4f position;
        Common::Vec4f color;
        Common::Vec2f tex_coord0;
        Common::Vec2f tex_coord1;
        Common::Vec2f tex_coord2;
        float tex_coord0_w;
        Common::Vec4f normquat;
        Common::Vec3f view;
    };

    struct VertexArrayInfo {
        u32 vs_input_index_min;
        u32 vs_input_index_max;
        u32 vs_input_size;
    };

    /// Retrieve the range and the size of the input vertex
    VertexArrayInfo AnalyzeVertexArray(bool is_indexed, u32 stride_alignment = 1);

protected:
    Memory::MemorySystem& memory;
    Pica::PicaCore& pica;
    Pica::RegsInternal& regs;

    std::vector<HardwareVertex> vertex_batch;
    Pica::Shader::UserConfig user_config{};
    bool shader_dirty = true;

    alignas(16) VSUniformBlockData vs_uniform_block_data{};
    alignas(16) FSUniformBlockData fs_uniform_block_data{};
    using LightLUT = std::array<Common::Vec2f, 256>;
    std::array<LightLUT, Pica::LightingRegs::NumLightingSampler> lighting_lut_data{};
    std::array<Common::Vec2f, 128> fog_lut_data{};
    std::array<Common::Vec2f, 128> proctex_noise_lut_data{};
    std::array<Common::Vec2f, 128> proctex_color_map_data{};
    std::array<Common::Vec2f, 128> proctex_alpha_map_data{};
    std::array<Common::Vec4f, 256> proctex_lut_data{};
    std::array<Common::Vec4f, 256> proctex_diff_lut_data{};
};

} // namespace VideoCore
