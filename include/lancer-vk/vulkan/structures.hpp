#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "./enums.hpp"
#include "./bitfields.hpp"

namespace vkh { // TODO: Coverage ALL of MOST and Common USING Vulkan Structures
                // TODO: WIP FULL C++20 SUPPORT

    // Structures should be packed accurately as Vulkan.H and Vulkan.HPP
    #pragma pack(push, 8) // BUT Vulkan Should PACK ONLY BY ONE BYTE

    // 
    #define STRUCT_OPERATORS(NAME)\
        operator ::NAME&() { return reinterpret_cast<::NAME&>(*this); };\
        operator const ::NAME&() const { return reinterpret_cast<const ::NAME&>(*this); };\
        operator ::NAME*() { return reinterpret_cast<::NAME*>(this); };\
        operator const ::NAME*() const { return reinterpret_cast<const ::NAME*>(this); };\
        NAME& operator =( const ::NAME& info ) { reinterpret_cast<::NAME&>(*this) = info; return *this; };\
        NAME& operator =( const NAME& info ) { reinterpret_cast<::NAME&>(*this) = reinterpret_cast<const ::NAME&>(info); return *this; };
        

    //#ifdef USE_GLM
    using VkExtent3D = glm::uvec3;
    using VkExtent2D = glm::uvec2;
    using VkOffset3D = glm::ivec3;
    using VkOffset2D = glm::ivec2;
    //#endif

    // 
    typedef struct VkRect2D {
        VkOffset2D offset = { 0,0 };
        VkExtent2D extent = { 1,1 };
        STRUCT_OPERATORS(VkRect2D)
    } VkRect2D;

    //
    typedef struct VkDescriptorPoolSize {
        VkDescriptorType    type            = VK_DESCRIPTOR_TYPE_SAMPLER;
        uint32_t            descriptorCount = 1u;
        STRUCT_OPERATORS(VkDescriptorPoolSize)
    } VkDescriptorPoolSize;

    // 
    typedef struct VkDescriptorPoolCreateInfo {
        VkStructureType                sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        const void*                    pNext            = nullptr;
        VkDescriptorPoolCreateFlags    flags            = {};
        uint32_t                       maxSets          = 256u;
        uint32_t                       poolSizeCount    = 0u;
        const VkDescriptorPoolSize*    pPoolSizes       = nullptr;
        STRUCT_OPERATORS(VkDescriptorPoolCreateInfo)
    } VkDescriptorPoolCreateInfo;

    // 
    typedef struct VkDeviceCreateInfo {
        VkStructureType                    sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        const void*                        pNext                    = nullptr;
        VkDeviceCreateFlags                flags                    = {};
        uint32_t                           queueCreateInfoCount     = 0u;
        const VkDeviceQueueCreateInfo*     pQueueCreateInfos        = nullptr;
        uint32_t                           enabledLayerCount        = 0u;
        const char* const*                 ppEnabledLayerNames      = nullptr;
        uint32_t                           enabledExtensionCount    = 0u;
        const char* const*                 ppEnabledExtensionNames  = nullptr;
        const VkPhysicalDeviceFeatures*    pEnabledFeatures         = nullptr;

        VkDeviceCreateInfo& setQueueCreateInfos(const std::vector<VkDeviceQueueCreateInfo>& V = {}) { pQueueCreateInfos = V.data(); queueCreateInfoCount = V.size(); return *this; };
        VkDeviceCreateInfo& setPEnabledExtensionNames(const std::vector<const char*>& V = {}) { ppEnabledExtensionNames = V.data(); enabledExtensionCount = V.size(); return *this; };
        VkDeviceCreateInfo& setPEnabledLayerNames(const std::vector<const char* >& V = {}) { ppEnabledLayerNames = V.data(); enabledLayerCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkDeviceCreateInfo)
    } VkDeviceCreateInfo;

    // 
    typedef struct VkBufferCreateInfo {
        VkStructureType     sType                   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        const void*         pNext                   = nullptr;
        VkBufferCreateFlags flags                   = {};
        VkDeviceSize        size                    = 4u;
        VkBufferUsageFlags  usage                   = {};
        VkSharingMode       sharingMode             = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t            queueFamilyIndexCount   = 0u;
        const uint32_t*     pQueueFamilyIndices     = nullptr;

        VkBufferCreateInfo& setQueueFamilyIndices(const std::vector<uint32_t>& V = {}) { pQueueFamilyIndices = V.data(); queueFamilyIndexCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkBufferCreateInfo)
    } VkBufferCreateInfo;

    // 
    typedef struct VkImageCreateInfo {
        VkStructureType          sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        const void*              pNext                  = nullptr;
        VkImageCreateFlags       flags                  = {};
        VkImageType              imageType              = VK_IMAGE_TYPE_1D;
        VkFormat                 format                 = VK_FORMAT_R8G8_UNORM;
        VkExtent3D               extent                 = {1u,1u,1u};
        uint32_t                 mipLevels              = 1u;
        uint32_t                 arrayLayers            = 1u;
        VkSampleCountFlagBits    samples                = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling            tiling                 = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags        usage                  = {};
        VkSharingMode            sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t                 queueFamilyIndexCount  = 0u;
        const uint32_t*          pQueueFamilyIndices    = nullptr;
        VkImageLayout            initialLayout          = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageCreateInfo& setQueueFamilyIndices(const std::vector<uint32_t>& V = {}) { pQueueFamilyIndices = V.data(); queueFamilyIndexCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkImageCreateInfo)
    } VkImageCreateInfo;

    // 
    typedef struct VkImageSubresourceRange {
        VkImageAspectFlags    aspectMask = { .eColor = 1 };
        uint32_t              baseMipLevel = 0u;
        uint32_t              levelCount = 1u;
        uint32_t              baseArrayLayer = 0u;
        uint32_t              layerCount = 1u;
        STRUCT_OPERATORS(VkImageSubresourceRange)
    } VkImageSubresourceRange;

    // 
    typedef struct VkComponentMapping {
        VkComponentSwizzle r = VK_COMPONENT_SWIZZLE_R;
        VkComponentSwizzle g = VK_COMPONENT_SWIZZLE_G;
        VkComponentSwizzle b = VK_COMPONENT_SWIZZLE_B;
        VkComponentSwizzle a = VK_COMPONENT_SWIZZLE_A;
        STRUCT_OPERATORS(VkComponentMapping)
    } VkComponentMapping;

    // 
    typedef struct VkImageViewCreateInfo {
        VkStructureType            sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        const void*                pNext            = nullptr;
        VkImageViewCreateFlags     flags            = {};
        VkImage                    image            = VK_NULL_HANDLE;
        VkImageViewType            viewType         = VK_IMAGE_VIEW_TYPE_2D;
        VkFormat                   format           = VK_FORMAT_R8G8B8A8_UNORM;
        VkComponentMapping         components       = {};
        VkImageSubresourceRange    subresourceRange = {};
        STRUCT_OPERATORS(VkImageViewCreateInfo)
    } VkImageViewCreateInfo;

    // 
    typedef struct VkPipelineInputAssemblyStateCreateInfo {
        VkStructureType                                 sType                   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        const void*                                     pNext                   = nullptr;
        VkPipelineInputAssemblyStateCreateFlags         flags                   = {};
        VkPrimitiveTopology                             topology                = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32                                        primitiveRestartEnable  = false;

        STRUCT_OPERATORS(VkPipelineInputAssemblyStateCreateInfo)
    } VkPipelineInputAssemblyStateCreateInfo;

    //
    typedef struct VkDescriptorSetAllocateInfo {
        VkStructureType                 sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        const void*                     pNext              = nullptr;
        VkDescriptorPool                descriptorPool     = VK_NULL_HANDLE;
        uint32_t                        descriptorSetCount = 1u;
        const VkDescriptorSetLayout*    pSetLayouts        = nullptr;

        VkDescriptorSetAllocateInfo& setSetLayouts(const std::vector<VkDescriptorSetLayout>& V = {}) { pSetLayouts = V.data(); descriptorSetCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkDescriptorSetAllocateInfo)
    } VkDescriptorSetAllocateInfo;

    // 
    typedef struct VkPipelineVertexInputStateCreateInfo {
        VkStructureType                                 sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        const void*                                     pNext                           = nullptr;
        VkPipelineVertexInputStateCreateFlags           flags                           = {};
        uint32_t                                        vertexBindingDescriptionCount   = 0u;
        const VkVertexInputBindingDescription*          pVertexBindingDescriptions      = nullptr;
        uint32_t                                        vertexAttributeDescriptionCount = 0u;
        const VkVertexInputAttributeDescription*        pVertexAttributeDescriptions    = nullptr;

        VkPipelineVertexInputStateCreateInfo& setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& V = {}) { pVertexBindingDescriptions = V.data(); vertexBindingDescriptionCount = V.size(); return *this; };
        VkPipelineVertexInputStateCreateInfo& setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& V = {}) { pVertexAttributeDescriptions = V.data(); vertexAttributeDescriptionCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkPipelineVertexInputStateCreateInfo)
    } VkPipelineVertexInputStateCreateInfo;

    // 
    typedef struct VkPipelineTessellationStateCreateInfo {
        VkStructureType                           sType                 = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        const void*                               pNext                 = nullptr;
        VkPipelineTessellationStateCreateFlags    flags                 = {};
        uint32_t                                  patchControlPoints    = 0u;

        STRUCT_OPERATORS(VkPipelineTessellationStateCreateInfo)
    } VkPipelineTessellationStateCreateInfo;

    // 
    typedef struct VkPipelineViewportStateCreateInfo {
        VkStructureType                       sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        const void*                           pNext         = nullptr;
        VkPipelineViewportStateCreateFlags    flags         = {};
        uint32_t                              viewportCount = 1u;
        const VkViewport*                     pViewports    = nullptr;
        uint32_t                              scissorCount  = 1u;
        const VkRect2D*                       pScissors     = nullptr;

        VkPipelineViewportStateCreateInfo& setViewports(const std::vector<VkViewport>& V = {}) { pViewports = V.data(); viewportCount = V.size(); return *this; };
        VkPipelineViewportStateCreateInfo& setScissors(const std::vector<VkRect2D>& V = {}) { pScissors = V.data(); scissorCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkPipelineViewportStateCreateInfo)
    } VkPipelineViewportStateCreateInfo;

    // 
    typedef struct VkPipelineRasterizationStateCreateInfo {
        VkStructureType                            sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        const void*                                pNext                    = nullptr;
        VkPipelineRasterizationStateCreateFlags    flags                    = {};
        VkBool32                                   depthClampEnable         = false;
        VkBool32                                   rasterizerDiscardEnable  = false;
        VkPolygonMode                              polygonMode              = VK_POLYGON_MODE_FILL;
        VkCullModeFlags                            cullMode                 = {};
        VkFrontFace                                frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE ;
        VkBool32                                   depthBiasEnable          = false;
        float                                      depthBiasConstantFactor  = 0.f;
        float                                      depthBiasClamp           = 0.f;
        float                                      depthBiasSlopeFactor     = 0.f;
        float                                      lineWidth                = 1.f;

        STRUCT_OPERATORS(VkPipelineRasterizationStateCreateInfo)
    } VkPipelineRasterizationStateCreateInfo;

    //
    typedef struct VkPipelineMultisampleStateCreateInfo {
        VkStructureType                          sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        const void*                              pNext                  = nullptr;
        VkPipelineMultisampleStateCreateFlags    flags                  = {};
        VkSampleCountFlagBits                    rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT;
        VkBool32                                 sampleShadingEnable    = false;
        float                                    minSampleShading       = 0.f;
        const VkSampleMask*                      pSampleMask            = nullptr;
        VkBool32                                 alphaToCoverageEnable  = false;
        VkBool32                                 alphaToOneEnable       = false;

        STRUCT_OPERATORS(VkPipelineMultisampleStateCreateInfo)
    } VkPipelineMultisampleStateCreateInfo;

    // 
    typedef struct VkStencilOpState {
        VkStencilOp    failOp       = VK_STENCIL_OP_KEEP;
        VkStencilOp    passOp       = VK_STENCIL_OP_KEEP;
        VkStencilOp    depthFailOp  = VK_STENCIL_OP_KEEP;
        VkCompareOp    compareOp    = VK_COMPARE_OP_ALWAYS;
        uint32_t       compareMask  = 0u;
        uint32_t       writeMask    = 0u;
        uint32_t       reference    = 0u;

        STRUCT_OPERATORS(VkStencilOpState)
    } VkStencilOpState;

    // 
    typedef struct VkPipelineDepthStencilStateCreateInfo {
        VkStructureType                           sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        const void*                               pNext                 = nullptr;
        VkPipelineDepthStencilStateCreateFlags    flags                 = {};
        VkBool32                                  depthTestEnable       = false;
        VkBool32                                  depthWriteEnable      = false;
        VkCompareOp                               depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
        VkBool32                                  depthBoundsTestEnable = false;
        VkBool32                                  stencilTestEnable     = false;
        VkStencilOpState                          front                 = {};
        VkStencilOpState                          back                  = {};
        float                                     minDepthBounds        = 0.f;
        float                                     maxDepthBounds        = 1.f;

        STRUCT_OPERATORS(VkPipelineDepthStencilStateCreateInfo)
    } VkPipelineDepthStencilStateCreateInfo;

    // 
    typedef struct VkPipelineColorBlendAttachmentState {
        VkBool32                 blendEnable         = false;
        VkBlendFactor            srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor            dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp                colorBlendOp        = VK_BLEND_OP_ADD;
        VkBlendFactor            srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor            dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp                alphaBlendOp        = VK_BLEND_OP_ADD;
        VkColorComponentFlags    colorWriteMask      = {.eR = 1, .eG = 1, .eB = 1, .eA = 1};
        STRUCT_OPERATORS(VkPipelineColorBlendAttachmentState)
    } VkPipelineColorBlendAttachmentState;

    // 
    typedef struct VkPipelineColorBlendStateCreateInfo {
        VkStructureType                               sType            = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        const void*                                   pNext            = nullptr;
        VkPipelineColorBlendStateCreateFlags          flags            = {};
        VkBool32                                      logicOpEnable    = false;
        VkLogicOp                                     logicOp          = VK_LOGIC_OP_SET;
        uint32_t                                      attachmentCount  = 0u;
        const VkPipelineColorBlendAttachmentState*    pAttachments     = nullptr;
        glm::vec4                                     blendConstants   = {1.f,1.f,1.f,1.f};
        //float                                         blendConstants[4];

        VkPipelineColorBlendStateCreateInfo& setAttachments(const std::vector<VkPipelineColorBlendAttachmentState>& V = {}) { pAttachments = V.data(); attachmentCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkPipelineColorBlendStateCreateInfo)
    } VkPipelineColorBlendStateCreateInfo;

    // 
    typedef struct VkPipelineDynamicStateCreateInfo {
        VkStructureType                      sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        const void*                          pNext              = nullptr;
        VkPipelineDynamicStateCreateFlags    flags              = {};
        uint32_t                             dynamicStateCount  = 0u;
        const VkDynamicState*                pDynamicStates     = nullptr;

        VkPipelineDynamicStateCreateInfo& setDynamicStates(const std::vector<VkDynamicState>& V = {}) { pDynamicStates = V.data(); dynamicStateCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkPipelineDynamicStateCreateInfo)
    } VkPipelineDynamicStateCreateInfo;

    //
    typedef struct VkPipelineShaderStageCreateInfo {
        VkStructureType                     sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        const void*                         pNext               = nullptr;
        VkPipelineShaderStageCreateFlags    flags               = {};
        VkShaderStageFlagBits               stage               = VK_SHADER_STAGE_COMPUTE_BIT;
        VkShaderModule                      module              = VK_NULL_HANDLE;
        const char*                         pName               = "main";
        const VkSpecializationInfo*         pSpecializationInfo = nullptr;

        STRUCT_OPERATORS(VkPipelineShaderStageCreateInfo)
    } VkPipelineShaderStageCreateInfo;

    // 
    typedef struct VkGraphicsPipelineCreateInfo {
        VkStructureType                                  sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        const void*                                      pNext                  = nullptr;
        VkPipelineCreateFlags                            flags                  = {};
        uint32_t                                         stageCount             = 0u;
        const VkPipelineShaderStageCreateInfo*           pStages                = nullptr;
        const VkPipelineVertexInputStateCreateInfo*      pVertexInputState      = nullptr;
        const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState    = nullptr;
        const VkPipelineTessellationStateCreateInfo*     pTessellationState     = nullptr;
        const VkPipelineViewportStateCreateInfo*         pViewportState         = nullptr;
        const VkPipelineRasterizationStateCreateInfo*    pRasterizationState    = nullptr;
        const VkPipelineMultisampleStateCreateInfo*      pMultisampleState      = nullptr;
        const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState     = nullptr;
        const VkPipelineColorBlendStateCreateInfo*       pColorBlendState       = nullptr;
        const VkPipelineDynamicStateCreateInfo*          pDynamicState          = nullptr;
        VkPipelineLayout                                 layout                 = VK_NULL_HANDLE;
        VkRenderPass                                     renderPass             = VK_NULL_HANDLE;
        uint32_t                                         subpass                = 0u;
        VkPipeline                                       basePipelineHandle     = VK_NULL_HANDLE;
        int32_t                                          basePipelineIndex      = 0;

        VkGraphicsPipelineCreateInfo& setStages(const std::vector<VkPipelineShaderStageCreateInfo>& V = {}) { pStages = V.data(); stageCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkGraphicsPipelineCreateInfo)
    } VkGraphicsPipelineCreateInfo;

    // 
    typedef struct VkComputePipelineCreateInfo {
        VkStructureType                    sType                = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        const void*                        pNext                = nullptr;
        VkPipelineCreateFlags              flags                = {};
        VkPipelineShaderStageCreateInfo    stage                = {};
        VkPipelineLayout                   layout               = VK_NULL_HANDLE;
        VkPipeline                         basePipelineHandle   = VK_NULL_HANDLE;
        int32_t                            basePipelineIndex    = 0;

        STRUCT_OPERATORS(VkComputePipelineCreateInfo)
    } VkComputePipelineCreateInfo;

    // 
    typedef struct VkRayTracingShaderGroupCreateInfoNV {
        VkStructureType                  sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        const void*                      pNext              = nullptr;
        VkRayTracingShaderGroupTypeNV    type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        uint32_t                         generalShader      = VK_SHADER_UNUSED_NV;
        uint32_t                         closestHitShader   = VK_SHADER_UNUSED_NV;
        uint32_t                         anyHitShader       = VK_SHADER_UNUSED_NV;
        uint32_t                         intersectionShader = VK_SHADER_UNUSED_NV;
        STRUCT_OPERATORS(VkRayTracingShaderGroupCreateInfoNV)
    } VkRayTracingShaderGroupCreateInfoNV;

    //
    typedef struct VkRayTracingPipelineCreateInfoNV {
        VkStructureType                               sType                 = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        const void*                                   pNext                 = nullptr;
        VkPipelineCreateFlags                         flags                 = {};
        uint32_t                                      stageCount            = 1u;
        const VkPipelineShaderStageCreateInfo*        pStages               = nullptr;
        uint32_t                                      groupCount            = 1u;
        const VkRayTracingShaderGroupCreateInfoNV*    pGroups               = nullptr;
        uint32_t                                      maxRecursionDepth     = 1u;
        VkPipelineLayout                              layout                = VK_NULL_HANDLE;
        VkPipeline                                    basePipelineHandle    = VK_NULL_HANDLE;
        int32_t                                       basePipelineIndex     = 0;

        VkRayTracingPipelineCreateInfoNV& setStages(const std::vector<VkPipelineShaderStageCreateInfo>& V = {}) { pStages = V.data(); stageCount = V.size(); return *this; };
        VkRayTracingPipelineCreateInfoNV& setGroups(const std::vector<VkRayTracingShaderGroupCreateInfoNV>& V = {}) { pGroups = V.data(); groupCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkRayTracingPipelineCreateInfoNV)
    } VkRayTracingPipelineCreateInfoNV;

    //
    typedef struct VkDescriptorSetLayoutBinding {
        uint32_t              binding               = 0u;
        VkDescriptorType      descriptorType        = VK_DESCRIPTOR_TYPE_SAMPLER;
        uint32_t              descriptorCount       = 1u;
        VkShaderStageFlags    stageFlags            = { .eCompute = 1 };
        const VkSampler*      pImmutableSamplers    = nullptr;
        STRUCT_OPERATORS(VkDescriptorSetLayoutBinding)
    } VkDescriptorSetLayoutBinding;

    // 
    typedef struct VkPipelineLayoutCreateInfo {
        VkStructureType                 sType                   = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        const void*                     pNext                   = nullptr;
        VkPipelineLayoutCreateFlags     flags                   = {};
        uint32_t                        setLayoutCount          = 0u;
        const VkDescriptorSetLayout*    pSetLayouts             = nullptr;
        uint32_t                        pushConstantRangeCount  = 0u;
        const VkPushConstantRange*      pPushConstantRanges     = nullptr;
        VkPipelineLayoutCreateInfo& setSetLayouts(const std::vector<VkDescriptorSetLayout>& V = {}) { pSetLayouts = V.data(); setLayoutCount = V.size(); return *this; };
        VkPipelineLayoutCreateInfo& setPushConstantRanges(const std::vector<VkPushConstantRange>& V = {}) { pPushConstantRanges = V.data(); pushConstantRangeCount = V.size(); return *this; };
        STRUCT_OPERATORS(VkPipelineLayoutCreateInfo)
    } VkPipelineLayoutCreateInfo;

    // 
    typedef struct VkDescriptorSetLayoutBindingFlagsCreateInfoEXT {
        VkStructureType                       sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        const void*                           pNext         = nullptr;
        uint32_t                              bindingCount  = 0u;
        const VkDescriptorBindingFlagsEXT*    pBindingFlags = {};

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& setBindingFlags(const std::vector<VkDescriptorBindingFlagsEXT>& V = {}) { pBindingFlags = V.data(); bindingCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkDescriptorSetLayoutBindingFlagsCreateInfoEXT)
    } VkDescriptorSetLayoutBindingFlagsCreateInfoEXT;

    // 
    typedef struct VkDescriptorUpdateTemplateEntry {
        uint32_t            dstBinding      = 0u;
        uint32_t            dstArrayElement = 0u;
        uint32_t            descriptorCount = 1u;
        VkDescriptorType    descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        size_t              offset          = 0u;
        size_t              stride          = 8u;

        STRUCT_OPERATORS(VkDescriptorUpdateTemplateEntry)
    } VkDescriptorUpdateTemplateEntry;

    // 
    typedef struct VkDescriptorUpdateTemplateCreateInfo {
        VkStructureType                           sType                         = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
        const void*                               pNext                         = nullptr;
        VkDescriptorUpdateTemplateCreateFlags     flags                         = {};
        uint32_t                                  descriptorUpdateEntryCount    = 0u;
        const VkDescriptorUpdateTemplateEntry*    pDescriptorUpdateEntries      = nullptr;
        VkDescriptorUpdateTemplateType            templateType                  = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
        VkDescriptorSetLayout                     descriptorSetLayout           = VK_NULL_HANDLE;
        VkPipelineBindPoint                       pipelineBindPoint             = VK_PIPELINE_BIND_POINT_COMPUTE;
        VkPipelineLayout                          pipelineLayout                = VK_NULL_HANDLE;
        uint32_t                                  set                           = 0u;

        VkDescriptorUpdateTemplateCreateInfo& setDescriptorUpdateEntries(const std::vector<VkDescriptorUpdateTemplateEntry>& V = {}) { pDescriptorUpdateEntries = V.data(); descriptorUpdateEntryCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkDescriptorUpdateTemplateCreateInfo)
    } VkDescriptorUpdateTemplateCreateInfo;

    // 
    typedef struct VkDescriptorSetLayoutCreateInfo {
        VkStructureType                        sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        const void*                            pNext        = nullptr;
        VkDescriptorSetLayoutCreateFlags       flags        = {};
        uint32_t                               bindingCount = 0u;
        const VkDescriptorSetLayoutBinding*    pBindings    = nullptr;

        VkDescriptorSetLayoutCreateInfo& setBindings(const std::vector<VkDescriptorSetLayoutBinding>& V = {}) { pBindings = V.data(); bindingCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkDescriptorSetLayoutCreateInfo)
    } VkDescriptorSetLayoutCreateInfo;

    // 
    typedef struct VkAttachmentDescription {
        VkAttachmentDescriptionFlags    flags           = {};
        VkFormat                        format          = VK_FORMAT_R8G8B8A8_UNORM;
        VkSampleCountFlagBits           samples         = VK_SAMPLE_COUNT_1_BIT;
        VkAttachmentLoadOp              loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp             storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        VkAttachmentLoadOp              stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp             stencilStoreOp  = VK_ATTACHMENT_STORE_OP_STORE;
        VkImageLayout                   initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout                   finalLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

        STRUCT_OPERATORS(VkAttachmentDescription)
    } VkAttachmentDescription;

    // 
    typedef struct VkSubpassDescription {
        VkSubpassDescriptionFlags       flags                   = {};
        VkPipelineBindPoint             pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        uint32_t                        inputAttachmentCount    = 0u;
        const VkAttachmentReference*    pInputAttachments       = nullptr;
        uint32_t                        colorAttachmentCount    = 0u;
        const VkAttachmentReference*    pColorAttachments       = nullptr;
        const VkAttachmentReference*    pResolveAttachments     = nullptr;
        const VkAttachmentReference*    pDepthStencilAttachment = nullptr;
        uint32_t                        preserveAttachmentCount = 0u;
        const uint32_t*                 pPreserveAttachments    = nullptr;

        VkSubpassDescription& setColorAttachments(const std::vector<VkAttachmentReference>& V = {}) { pColorAttachments = V.data(); colorAttachmentCount = V.size(); return *this; };
        VkSubpassDescription& setInputAttachments(const std::vector<VkAttachmentReference>& V = {}) { pInputAttachments = V.data(); inputAttachmentCount = V.size(); return *this; };
        VkSubpassDescription& setPreserveAttachments(const std::vector<uint32_t>& V = {}) { pPreserveAttachments = V.data(); preserveAttachmentCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkSubpassDescription)
    } VkSubpassDescription;

    // 
    typedef struct VkSubpassDependency {
        uint32_t                srcSubpass      = 0u;
        uint32_t                dstSubpass      = 0u;
        VkPipelineStageFlags    srcStageMask    = {};
        VkPipelineStageFlags    dstStageMask    = {};
        VkAccessFlags           srcAccessMask   = {};
        VkAccessFlags           dstAccessMask   = {};
        VkDependencyFlags       dependencyFlags = {};

        STRUCT_OPERATORS(VkSubpassDependency)
    } VkSubpassDependency;

    // 
    typedef struct VkRenderPassCreateInfo {
        VkStructureType                   sType             = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        const void*                       pNext             = nullptr;
        VkRenderPassCreateFlags           flags             = {};
        uint32_t                          attachmentCount   = 0u;
        const VkAttachmentDescription*    pAttachments      = nullptr;
        uint32_t                          subpassCount      = 0u;
        const VkSubpassDescription*       pSubpasses        = nullptr;
        uint32_t                          dependencyCount   = 0u;
        const VkSubpassDependency*        pDependencies     = nullptr;

        VkRenderPassCreateInfo& setAttachments(const std::vector<VkAttachmentDescription>& V = {}) { pAttachments = V.data(); attachmentCount = V.size(); return *this; };
        VkRenderPassCreateInfo& setSubpasses(const std::vector<VkSubpassDescription>& V = {}) { pSubpasses = V.data(); subpassCount = V.size(); return *this; };
        VkRenderPassCreateInfo& setDependencies(const std::vector<VkSubpassDependency>& V = {}) { pDependencies = V.data(); dependencyCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkRenderPassCreateInfo)
    } VkRenderPassCreateInfo;

    // 
    typedef struct VkAccelerationStructureMemoryRequirementsInfoNV {
        VkStructureType                                    sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        const void*                                        pNext                    = nullptr;
        VkAccelerationStructureMemoryRequirementsTypeNV    type                     = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
        VkAccelerationStructureNV                          accelerationStructure    = VK_NULL_HANDLE;
        STRUCT_OPERATORS(VkAccelerationStructureMemoryRequirementsInfoNV)
    } VkAccelerationStructureMemoryRequirementsInfoNV;

    // 
    typedef struct VkGeometryAABBNV {
        VkStructureType    sType    = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
        const void*        pNext    = nullptr;
        VkBuffer           aabbData = VK_NULL_HANDLE;
        uint32_t           numAABBs = 0u;
        uint32_t           stride   = 24u;
        VkDeviceSize       offset   = 0u;
        STRUCT_OPERATORS(VkGeometryAABBNV)
    } VkGeometryAABBNV;

    // 
    typedef struct VkGeometryTrianglesNV {
        VkStructureType    sType            = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        const void*        pNext            = nullptr;
        VkBuffer           vertexData       = VK_NULL_HANDLE;
        VkDeviceSize       vertexOffset     = 0u;
        uint32_t           vertexCount      = 1u;
        VkDeviceSize       vertexStride     = 16u;
        VkFormat           vertexFormat     = VK_FORMAT_R32G32B32_SFLOAT;
        VkBuffer           indexData        = VK_NULL_HANDLE;
        VkDeviceSize       indexOffset      = 0u;
        uint32_t           indexCount       = 0u;
        VkIndexType        indexType        = VK_INDEX_TYPE_NONE_NV;
        VkBuffer           transformData    = VK_NULL_HANDLE;
        VkDeviceSize       transformOffset  = 0u;
        STRUCT_OPERATORS(VkGeometryTrianglesNV)
    } VkGeometryTrianglesNV;

    // 
    typedef struct VkGeometryDataNV {
        VkGeometryTrianglesNV    triangles = {};
        VkGeometryAABBNV         aabbs     = {};
    } VkGeometryDataNV;

    // 
    typedef struct VkGeometryNV {
        VkStructureType      sType        = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        const void*          pNext        = nullptr;
        VkGeometryTypeNV     geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        VkGeometryDataNV     geometry     = {};
        VkGeometryFlagsNV    flags        = {};
        STRUCT_OPERATORS(VkGeometryNV)
    } VkGeometryNV;

    // 
    typedef struct VkAccelerationStructureInfoNV {
        VkStructureType                        sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        const void*                            pNext         = nullptr;
        VkAccelerationStructureTypeNV          type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        VkBuildAccelerationStructureFlagsNV    flags         = {};
        uint32_t                               instanceCount = 0u;
        uint32_t                               geometryCount = 0u;
        const VkGeometryNV*                    pGeometries   = nullptr;

        VkAccelerationStructureInfoNV& setGeometries(const std::vector<VkGeometryNV>& V = {}) { pGeometries = V.data(); geometryCount = V.size(); return *this; };

        STRUCT_OPERATORS(VkAccelerationStructureInfoNV)
    } VkAccelerationStructureInfoNV;

    // 
    typedef struct VkAccelerationStructureCreateInfoNV {
        VkStructureType                  sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
        const void*                      pNext           = nullptr;
        VkDeviceSize                     compactedSize   = 0u;
        VkAccelerationStructureInfoNV    info            = {};
        STRUCT_OPERATORS(VkAccelerationStructureCreateInfoNV)
    } VkAccelerationStructureCreateInfoNV;

    #pragma pack(pop)


    // CODING EXTRAS...
    #pragma pack(push, 1)
    struct VsGeometryInstance {
        glm::mat3x4 transform = {
            glm::vec4(1.0f,0.0f,0.0f,0.0f),
            glm::vec4(0.0f,1.0f,0.0f,0.0f),
            glm::vec4(0.0f,0.0f,1.0f,0.0f)
        };
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        union { uint32_t flags_8u : 8; VkGeometryInstanceFlagsNV flags = {}; };
        uint64_t accelerationStructureHandle = 0ull;
        //STRUCT_OPERATORS(VsGeometryInstance) // NO NATIVE TYPE
    };
    #pragma pack(pop)

};
