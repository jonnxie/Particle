//
// Created by maybe on 2021/6/6.
//
#include "precompiledhead.h"

#include "shaderpool.h"
#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Object/device.h"
#include <mutex>
#include ConfigCatalog
#include <algorithm>

using namespace Shatter::render;

static bool if_created = false;
static std::mutex pool_mutex;

ShaderPool &ShaderPool::getPool() {
    static ShaderPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!if_created){
        pool.init();
        if_created = true;
    }
    return pool;
}

static uint32_t FormatSize(VkFormat format)
{
    uint32_t result = 0;
    switch (format) {
        case VK_FORMAT_UNDEFINED: result = 0; break;
        case VK_FORMAT_R4G4_UNORM_PACK8: result = 1; break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B5G6R5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R8_UNORM: result = 1; break;
        case VK_FORMAT_R8_SNORM: result = 1; break;
        case VK_FORMAT_R8_USCALED: result = 1; break;
        case VK_FORMAT_R8_SSCALED: result = 1; break;
        case VK_FORMAT_R8_UINT: result = 1; break;
        case VK_FORMAT_R8_SINT: result = 1; break;
        case VK_FORMAT_R8_SRGB: result = 1; break;
        case VK_FORMAT_R8G8_UNORM: result = 2; break;
        case VK_FORMAT_R8G8_SNORM: result = 2; break;
        case VK_FORMAT_R8G8_USCALED: result = 2; break;
        case VK_FORMAT_R8G8_SSCALED: result = 2; break;
        case VK_FORMAT_R8G8_UINT: result = 2; break;
        case VK_FORMAT_R8G8_SINT: result = 2; break;
        case VK_FORMAT_R8G8_SRGB: result = 2; break;
        case VK_FORMAT_R8G8B8_UNORM: result = 3; break;
        case VK_FORMAT_R8G8B8_SNORM: result = 3; break;
        case VK_FORMAT_R8G8B8_USCALED: result = 3; break;
        case VK_FORMAT_R8G8B8_SSCALED: result = 3; break;
        case VK_FORMAT_R8G8B8_UINT: result = 3; break;
        case VK_FORMAT_R8G8B8_SINT: result = 3; break;
        case VK_FORMAT_R8G8B8_SRGB: result = 3; break;
        case VK_FORMAT_B8G8R8_UNORM: result = 3; break;
        case VK_FORMAT_B8G8R8_SNORM: result = 3; break;
        case VK_FORMAT_B8G8R8_USCALED: result = 3; break;
        case VK_FORMAT_B8G8R8_SSCALED: result = 3; break;
        case VK_FORMAT_B8G8R8_UINT: result = 3; break;
        case VK_FORMAT_B8G8R8_SINT: result = 3; break;
        case VK_FORMAT_B8G8R8_SRGB: result = 3; break;
        case VK_FORMAT_R8G8B8A8_UNORM: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SNORM: result = 4; break;
        case VK_FORMAT_R8G8B8A8_USCALED: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SSCALED: result = 4; break;
        case VK_FORMAT_R8G8B8A8_UINT: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SINT: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SRGB: result = 4; break;
        case VK_FORMAT_B8G8R8A8_UNORM: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SNORM: result = 4; break;
        case VK_FORMAT_B8G8R8A8_USCALED: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SSCALED: result = 4; break;
        case VK_FORMAT_B8G8R8A8_UINT: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SINT: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SRGB: result = 4; break;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: result = 4; break;
        case VK_FORMAT_R16_UNORM: result = 2; break;
        case VK_FORMAT_R16_SNORM: result = 2; break;
        case VK_FORMAT_R16_USCALED: result = 2; break;
        case VK_FORMAT_R16_SSCALED: result = 2; break;
        case VK_FORMAT_R16_UINT: result = 2; break;
        case VK_FORMAT_R16_SINT: result = 2; break;
        case VK_FORMAT_R16_SFLOAT: result = 2; break;
        case VK_FORMAT_R16G16_UNORM: result = 4; break;
        case VK_FORMAT_R16G16_SNORM: result = 4; break;
        case VK_FORMAT_R16G16_USCALED: result = 4; break;
        case VK_FORMAT_R16G16_SSCALED: result = 4; break;
        case VK_FORMAT_R16G16_UINT: result = 4; break;
        case VK_FORMAT_R16G16_SINT: result = 4; break;
        case VK_FORMAT_R16G16_SFLOAT: result = 4; break;
        case VK_FORMAT_R16G16B16_UNORM: result = 6; break;
        case VK_FORMAT_R16G16B16_SNORM: result = 6; break;
        case VK_FORMAT_R16G16B16_USCALED: result = 6; break;
        case VK_FORMAT_R16G16B16_SSCALED: result = 6; break;
        case VK_FORMAT_R16G16B16_UINT: result = 6; break;
        case VK_FORMAT_R16G16B16_SINT: result = 6; break;
        case VK_FORMAT_R16G16B16_SFLOAT: result = 6; break;
        case VK_FORMAT_R16G16B16A16_UNORM: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SNORM: result = 8; break;
        case VK_FORMAT_R16G16B16A16_USCALED: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SSCALED: result = 8; break;
        case VK_FORMAT_R16G16B16A16_UINT: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SINT: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SFLOAT: result = 8; break;
        case VK_FORMAT_R32_UINT: result = 4; break;
        case VK_FORMAT_R32_SINT: result = 4; break;
        case VK_FORMAT_R32_SFLOAT: result = 4; break;
        case VK_FORMAT_R32G32_UINT: result = 8; break;
        case VK_FORMAT_R32G32_SINT: result = 8; break;
        case VK_FORMAT_R32G32_SFLOAT: result = 8; break;
        case VK_FORMAT_R32G32B32_UINT: result = 12; break;
        case VK_FORMAT_R32G32B32_SINT: result = 12; break;
        case VK_FORMAT_R32G32B32_SFLOAT: result = 12; break;
        case VK_FORMAT_R32G32B32A32_UINT: result = 16; break;
        case VK_FORMAT_R32G32B32A32_SINT: result = 16; break;
        case VK_FORMAT_R32G32B32A32_SFLOAT: result = 16; break;
        case VK_FORMAT_R64_UINT: result = 8; break;
        case VK_FORMAT_R64_SINT: result = 8; break;
        case VK_FORMAT_R64_SFLOAT: result = 8; break;
        case VK_FORMAT_R64G64_UINT: result = 16; break;
        case VK_FORMAT_R64G64_SINT: result = 16; break;
        case VK_FORMAT_R64G64_SFLOAT: result = 16; break;
        case VK_FORMAT_R64G64B64_UINT: result = 24; break;
        case VK_FORMAT_R64G64B64_SINT: result = 24; break;
        case VK_FORMAT_R64G64B64_SFLOAT: result = 24; break;
        case VK_FORMAT_R64G64B64A64_UINT: result = 32; break;
        case VK_FORMAT_R64G64B64A64_SINT: result = 32; break;
        case VK_FORMAT_R64G64B64A64_SFLOAT: result = 32; break;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: result = 4; break;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: result = 4; break;

        default:
            break;
    }
    return result;
}

void ShaderPool::createShaderStage(const Shader_id& _id,const std::string& _filename,ShaderType _type){
    checkMap(m_map);

    if(!file::fileExists(_filename)){
        throwFile(_filename);
    }
    auto shader_code = file::readFile(_filename);
    VkPipelineShaderStageCreateInfo shader_info;
    shader_info.pNext = VK_NULL_HANDLE;
    shader_info.flags = 0;
    shader_info.pSpecializationInfo = VK_NULL_HANDLE;
    auto m_shader_module = createShaderModule(shader_code, *ShatterRender::getRender().getDevice());

    {
        SpvReflectShaderModule module = {};
        SpvReflectResult result = spvReflectCreateShaderModule(shader_code.size(), shader_code.data(), &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        /*
         * Generate Vertex Input Layout Create Info.
         */
        if (_type == VertexShader)
        {
            uint32_t count = 0;
            result = spvReflectEnumerateInputVariables(&module, &count, NULL);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectInterfaceVariable*> input_vars(count);
            result = spvReflectEnumerateInputVariables(&module, &count, input_vars.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
            if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
                // Demonstrates how to generate all necessary data structures to populate
                // a VkPipelineVertexInputStateCreateInfo structure, given the module's
                // expected input variables.
                //
                // Simplifying assumptions:
                // - All vertex input attributes are sourced from a single vertex buffer,
                //   bound to VB slot 0.
                // - Each vertex's attribute are laid out in ascending order by location.
                // - The format of each attribute matches its usage in the shader;
                //   float4 -> VK_FORMAT_R32G32B32A32_FLOAT, etc. No attribute compression is applied.
                // - All attributes are provided per-vertex, not per-instance.
                VkVertexInputBindingDescription binding_description = {};
                binding_description.binding = 0;
                binding_description.stride = 0;  // computed below
                binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
                std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
                attribute_descriptions.reserve(input_vars.size());
                for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
                    const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
                    // ignore built-in variables
                    if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                        continue;
                    }
                    VkVertexInputAttributeDescription attr_desc{};
                    attr_desc.location = refl_var.location;
                    attr_desc.binding = binding_description.binding;
                    attr_desc.format = static_cast<VkFormat>(refl_var.format);
                    attr_desc.offset = 0;  // final offset computed below after sorting.
                    attribute_descriptions.push_back(attr_desc);
                }
                // Sort attributes by location
                std::sort(std::begin(attribute_descriptions), std::end(attribute_descriptions),
                          [](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b) {
                              return a.location < b.location; });
                // Compute final offsets of each attribute, and total vertex stride.
                for (auto& attribute : attribute_descriptions) {
                    uint32_t format_size = FormatSize(attribute.format);
                    attribute.offset = binding_description.stride;
                    binding_description.stride += format_size;
                }
                // Nothing further is done with attribute_descriptions or binding_description
                // in this sample. A real application would probably derive this information from its
                // mesh format(s); a similar mechanism could be used to ensure mesh/shader compatibility.
            }
        }

        /*
         * Generate Descriptor Set Layout.
         */
        {
            uint32_t count = 0;
            result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorSet*> sets(count);
            result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
            std::vector<VkDescriptorSetLayout> vk_layouts(sets.size());
            VkDescriptorSetLayout vk_layout;
            for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
                const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
                DescriptorSetLayoutData& layout = set_layouts[i_set];
                layout.bindings.resize(refl_set.binding_count);
                for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
                    const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
                    VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
                    layout_binding.binding = refl_binding.binding;
                    layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                    layout_binding.descriptorCount = 1;
                    for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                        layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                    }
                    layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
                }
                layout.set_number = refl_set.set;
                layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layout.create_info.bindingCount = refl_set.binding_count;
                layout.create_info.pBindings = layout.bindings.data();

                vkCreateDescriptorSetLayout(SingleDevice(),
                                            &layout.create_info,
                                            nullptr,
                                            &vk_layout
                );
                vk_layouts[i_set] = vk_layout;
            }

            reflectSlMap[_id] = vk_layouts;
        }


        spvReflectDestroyShaderModule(&module);
    }

    shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    switch (_type) {
        case VertexShader:{
            shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        }
        case FragmentShader:{
            shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        }
        case ComputeShader:{
            shader_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        }
        case RayGen:{
            shader_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            break;
        }
        case RayMiss:{
            shader_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            break;
        }
        case RayClosest:{
            shader_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            break;
        }
        case TaskShader:{
            shader_info.stage = VK_SHADER_STAGE_TASK_BIT_NV;
            break;
        }
        case MeshShader:{
            shader_info.stage = VK_SHADER_STAGE_MESH_BIT_NV;
            break;
        }
        default: break;
    }
    shader_info.module = m_shader_module;
    shader_info.pName = "main";

    m_map[_id] = shader_info;
}

void ShaderPool::init() {
    createShaderStage("build_vs",tool::combineShader("g/gbuffer_vert.spv"), VertexShader);
    createShaderStage("build_fs",tool::combineShader("g/gbuffer_frag.spv"), FragmentShader);

    createShaderStage("agbasic_vs",tool::combineShader("agbasic/vert.spv"), VertexShader);
    createShaderStage("agbasic_fs",tool::combineShader("agbasic/frag.spv"), FragmentShader);

    createShaderStage("aginstance_vs",tool::combineShader("aginstance/vert.spv"), VertexShader);
    createShaderStage("aginstance_fs",tool::combineShader("aginstance/frag.spv"), FragmentShader);
    createShaderStage("aginstanceTex_fs",tool::combineShader("aginstance/tex_frag.spv"), FragmentShader);

    createShaderStage("gplanet_vs",tool::combineShader("gplanet/vert.spv"), VertexShader);
    createShaderStage("gplanet_fs",tool::combineShader("gplanet/frag.spv"), FragmentShader);

    createShaderStage("plane_vs",tool::combineShader("Plane/vert.spv"), VertexShader);
    createShaderStage("plane_fs",tool::combineShader("Plane/frag.spv"), FragmentShader);
    createShaderStage("planeTex_fs",tool::combineShader("Plane/tex_frag.spv"), FragmentShader);

    createShaderStage("cube_vs",tool::combineShader("cube/vert.spv"), VertexShader);
    createShaderStage("cube_fs",tool::combineShader("cube/frag.spv"), FragmentShader);
    createShaderStage("cubeTex_fs",tool::combineShader("cube/tex_frag.spv"), FragmentShader);

    createShaderStage("transparent_vs",tool::combineShader("pass/transparent_vert.spv"), VertexShader);
    createShaderStage("transparent_fs",tool::combineShader("pass/transparent_frag.spv"), FragmentShader);

    createShaderStage("composition_vs",tool::combineShader("pass/composition_vert.spv"), VertexShader);
    createShaderStage("composition_fs",tool::combineShader("pass/composition_frag.spv"), FragmentShader);

    createShaderStage("shadowDepth_vs",tool::combineShader("cascade_shadow/depthpass_vert.spv"), VertexShader);
    createShaderStage("shadowDepth_fs",tool::combineShader("cascade_shadow/depthpass_frag.spv"), FragmentShader);

    createShaderStage("polyline_vs",tool::combineShader("polyline/pc_vert.spv"), VertexShader);
    createShaderStage("polyline_fs",tool::combineShader("polyline/pc_frag.spv"), FragmentShader);

    createShaderStage("simple_ms",tool::combineShader("TaskMesh/mesh.spv"), MeshShader);

    createShaderStage("particle_vs", tool::combineShader("point/vert.spv"), VertexShader);
    createShaderStage("particle_fs", tool::combineShader("point/frag.spv"), FragmentShader);
    createShaderStage("particleTex_fs", tool::combineShader("point/tex_frag.spv"), FragmentShader);

    createShaderStage("planet_vs", tool::combineShader("planet/vert.spv"), VertexShader);
    createShaderStage("planet_fs", tool::combineShader("planet/frag.spv"), FragmentShader);

    createShaderStage("abasic_vs", tool::combineShader("abasic/vert.spv"), VertexShader);
    createShaderStage("abasic_fs", tool::combineShader("abasic/frag.spv"), FragmentShader);

    createShaderStage("gskin_vs", tool::combineShader("gskin/vert.spv"), VertexShader);
    createShaderStage("gskin_fs", tool::combineShader("gskin/frag.spv"), FragmentShader);

    createShaderStage("gskinInstance_vs", tool::combineShader("gskinInstance/vert.spv"), VertexShader);
    createShaderStage("gskinInstance_fs", tool::combineShader("gskinInstance/frag.spv"), FragmentShader);

    createShaderStage("aabb_vs", tool::combineShader("aabb/vert.spv"), VertexShader);
    createShaderStage("aabb_fs", tool::combineShader("aabb/frag.spv"), FragmentShader);

    createShaderStage("earth_vs", tool::combineShader("earth/vert.spv"), VertexShader);
    createShaderStage("earth_fs", tool::combineShader("earth/frag.spv"), FragmentShader);

    createShaderStage("earthHeight_vs", tool::combineShader("earthHeight/vert.spv"), VertexShader);
    createShaderStage("earthHeight_fs", tool::combineShader("earthHeight/frag.spv"), FragmentShader);

    /*
     * Skybox
     */
    {
        createShaderStage("skybox_vs",tool::combineShader("skybox/vert.spv"),VertexShader);
        createShaderStage("skybox_fs",tool::combineShader("skybox/frag.spv"),FragmentShader);
    }

    /*
     * imgui
     */
    {
        createShaderStage("gui_vs",tool::combineShader("gui/vert.spv"),VertexShader);
        createShaderStage("gui_fs",tool::combineShader("gui/frag.spv"),FragmentShader);
    }

    createShaderStage("quad_vs",tool::combineShader("quad/vert.spv"),VertexShader);
    createShaderStage("quad_fs",tool::combineShader("quad/frag.spv"),FragmentShader);

    createShaderStage("pbr_vs",tool::combineShader("pbrbasic/vert.spv"),VertexShader);
    createShaderStage("pbr_fs",tool::combineShader("pbrbasic/frag.spv"),FragmentShader);

    createShaderStage("animation_vs",tool::combineShader("animation/vert.spv"),VertexShader);
    createShaderStage("animation_fs",tool::combineShader("animation/frag.spv"),FragmentShader);

    createShaderStage("present_vs",tool::combineShader("present/vert.spv"),VertexShader);
    createShaderStage("present_fs",tool::combineShader("present/frag.spv"),FragmentShader);

}

void ShaderPool::release() {
    for(auto & i: m_map){
        vkDestroyShaderModule(Device::getDevice()(), i.second.module, nullptr);
    }

    for (auto & i : reflectSlMap) {
        for (auto& l : i.second) {
            vkDestroyDescriptorSetLayout(Device::getDevice()(), l, nullptr);
        }
    }
}

std::unordered_map<Shader_id, std::vector<VKSL>> &ShaderPool::getSLMap() {
    return reflectSlMap;
}




