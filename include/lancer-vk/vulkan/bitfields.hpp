#include <vulkan/vulkan.h>

namespace vkh {

    #define OPERATORS(NAME,BITS,COMP) \
        operator COMP& () { return reinterpret_cast<COMP&>(*this); };\
        operator const COMP& () const { return reinterpret_cast<const COMP&>(*this); };\
        NAME& operator=(const NAME& F) { (COMP&)(*this) = (COMP&)F; return *this; };\
        NAME& operator=(const COMP& F) { (COMP&)(*this) = F; return *this; };\
        NAME operator|(const NAME& F) { auto f = COMP(F) | COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator&(const NAME& F) { auto f = COMP(F) & COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator^(const NAME& F) { auto f = COMP(F) ^ COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator|(const BITS& F) { auto f = COMP(F) | COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator&(const BITS& F) { auto f = COMP(F) & COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator^(const BITS& F) { auto f = COMP(F) ^ COMP(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator~() { auto f = ~COMP(*this); return reinterpret_cast<NAME&>(f); };\
        operator BITS() {return reinterpret_cast<BITS&>(*this);};\
        operator const BITS&() const {return reinterpret_cast<const BITS&>(*this);};

    struct VkBufferCreateFlags { ::VkFlags
        eSparseBinding : 1,
        eResidency : 1,
        eAlised : 1,
        eProtected : 1,
        eDeviceAddressCaptureReplay : 1;

        OPERATORS(VkBufferCreateFlags,::VkBufferCreateFlagBits,::VkFlags)
    };

    struct VkBufferUsageFlags { ::VkFlags
        eTransferSrc : 1,
        eTransferDst : 1,
        eUniformTexelBuffer : 1,
        eStorageTexelBuffer : 1,
        eUniformBuffer : 1,
        eStorageBuffer : 1,
        eIndexBuffer : 1,
        eVertexBuffer : 1,
        eIndirectBuffer : 1,
        eConditionalRendering : 1,
        eRayTracingNV : 1,
        eTransformFeedbackBuffer : 1,
        eTransformFeedbackCounterBuffer : 1,
        eSharedDeviceAddress : 1;

        OPERATORS(VkBufferUsageFlags,::VkBufferUsageFlagBits,::VkFlags)
    };

    struct VkSampleCountFlags { ::VkFlags
        e1:1,
        e2:1,
        e4:1,
        e8:1,
        e16:1,
        e32:1,
        e64:1;
        
        OPERATORS(VkSampleCountFlags,::VkSampleCountFlagBits,::VkFlags)
    };

    struct VkImageCreateFlags { ::VkFlags
        eSparseBinding:1,
        eSparseResidency:1,
        eSparseAliased:1,
        eMutableFormat:1,
        eCubeCompatible:1,
        e2DArrayCompatible:1,
        eBlockTexelViewCompatible:1,
        eExtendedUsage:1,
        eDisjoint:1,
        eAlias:1,
        eProtected:1,
        eSampleLocationsCompatibleDepth:1,
        eCornerSampled:1,
        eSubsampled:1;
        
        OPERATORS(VkImageCreateFlags,::VkImageCreateFlagBits,::VkFlags);
    };

    struct VkImageUsageFlags { ::VkFlags
        eTransferSrc:1,
        eTransferDst:1,
        eSampled:1,
        eStorage:1,
        eColorAttachment:1,
        eDepthStencilAttachment:1,
        eTransientAttachment:1,
        eInputAttachment:1,
        eShadingRateImage:1,
        eFragmentDensityMap:1;

        OPERATORS(VkImageUsageFlags,::VkImageUsageFlagBits,::VkFlags);
    };

    struct VkPipelineStageFlags { ::VkFlags
        eTopOfPipe:1,
        eDrawIndirect:1,
        eVertexInput:1,
        eVertexShader:1,
        eTessellationControlShader:1,
        eTessellationEvaluationShader:1,
        eGeometryShader:1,
        eFragmentShader:1,
        eEarlyFragmentTests:1,
        eLateFragmentTests:1,
        eColorAttachment:1,
        eComputeShader:1,
        eTransfer:1,
        eBottomOfPipe:1,
        eHost:1,
        eAllGraphics:1,
        eAllCommands:1,
        eCommandProcess:1,
        eConditionalRendering:1,
        eTaskShader:1,
        eMeshShader:1,
        eRayTracing:1,
        eShadingRateImage;1,
        eFragmentDensityProcess:1,
        eTransformFeedback:1,
        eAccelerationStructureBuild:1;

        OPERATORS(VkPipelineStageFlags,::VkPipelineStageFlagBits,::VkFlags);
    };

    struct VkShaderStageFlags { ::VkFlags
        eVertex:1,
        eTessellationControl:1,
        eTessellationEvaluation:1,
        eGeometry:1,
        eFragment:1,
        eCompute:1,
        eTask:1,
        eMesh:1,
        eRaygen:1,
        eAnyHit:1,
        eClosestHit:1,
        eMiss:1,
        eIntersection:1,
        eCallable:1;

        OPERATORS(VkShaderStageFlags,::VkShaderStageFlagBits,::VkFlags);
    };

    struct VkCullModeFlags { ::VkFlags
        eFront:1,
        eBack:1;

        OPERATORS(VkCullModeFlags,::VkCullModeFlagBits,::VkFlags);
    };

    struct VkAccessFlags { ::VkFlags
        eIndirectCommandRead:1,
        eIndexRead:1,
        eVertexAttributeRead:1,
        eUniformRead:1,
        eInputAttachmentRead:1,
        eShaderRead:1,
        eShaderWrite:1,
        eColorAttachmentRead:1,
        eColorAttachmentWrite:1,
        eDepthStencilAttachmentRead:1,
        eDepthStencilAttachmentWrite:1,
        eTransferSrc:1,
        eTransferDst:1,
        eHostRead:1,
        eHostWrite:1,
        eMemoryRead:1,
        eMemoryWrite:1,
        eCommandProcessRead:1,
        eCommandProcessWrite:1,
        eColorAttachmentReadNoncoherent:1,
        eConditionalRenderingRead:1,
        eAccelerationStructureRead:1,
        aAccelerationStructureWrite:1,
        eShadingRateImageRead:1,
        eFragmentDensityMapRead:1,
        eTransformFeedbackWrite:1,
        eTransformFeedbackCounterRead:1,
        eTransformFeedbackCounterWrite:1;

        OPERATORS(VkAccessFlags,::VkAccessFlagBits,::VkFlags);
    };

    struct VkFormatFeatureFlags { ::VkFlags
        eSampledImage:1,
        eStorageImage:1,
        eStorageImageAtomic:1,
        eUniformTexelBuffer:1,
        eStorageTexelBuffer:1,
        eStorageTexelBufferAtomic:1,
        eVertexBuffer:1,
        eColorAttachment:1,
        eColorAttachmentBlend:1,
        eDepthStencilAttachment:1,
        eBlitSrc:1,
        eBlitDst:1,
        eSampledImageFilterLinear:1,
        eSampledImageFilterCubic:1,
        eTransferSrc:1,
        eTransferDst:1,
        eSampledImageFilterMinmax:1,
        eMidpointChromaSamples:1,
        eSampledImageYcbcrConversionLinearFilter:1,
        eSampledImageYcbcrConversionSeparateReconstructionFilter:1,
        eSampledImageYcbcrConversionChromaReconstructionExplicit:1,
        eSampledImageYcbcrConversionChromaReconstructionExplicitForceable:1,
        eDisjoint:1,
        eCositedChromaSamples:1,
        eFragmentDensityMap:1;

        OPERATORS(VkFormatFeatureFlags,::VkFormatFeatureFlagBits,::VkFlags);
    };

    struct VkColorComponentFlags { ::VkFlags
        eR:1,
        eG:1,
        eB:1,
        eA:1;
        
        OPERATORS(VkColorComponentFlags,::VkColorComponentFlagBits,::VkFlags);
    };

    struct VkDescriptorBindingFlags { ::VkFlags
        eUpdateAfterBind:1,
        eUpdateUnusedWhilePending:1,
        ePartiallyBound:1,
        eVariableDescriptorCount:1;
        
        OPERATORS(VkDescriptorBindingFlags,::VkDescriptorBindingFlagBits,::VkFlags);
    };

    struct VkBuildAccelerationStructureFlagsNV { ::VkFlags
        eAllowUpdate:1,
        eAllowCompaction:1,
        ePreferFastTrace:1,
        ePreferFastBuild:1,
        eLowMemory:1;
        
        OPERATORS(VkBuildAccelerationStructureFlagsNV,::VkBuildAccelerationStructureFlagBitsNV,::VkFlags);
    };

    struct VkGeometryFlagsNV { ::VkFlags
        eOpaque:1,
        eNoDuplicateAnyHitInvocation:1;
        
        OPERATORS(VkGeometryFlagsNV,::VkGeometryFlagBitsNV,::VkFlags);
    };

    struct VkQueueFlags { ::VkFlags
        eGraphics:1,
        eCompute:1,
        eTransfer:1,
        eSparseBinding:1,
        eProtected:1;
        
        OPERATORS(VkQueueFlags,::VkQueueFlagBits,::VkFlags);
    };

    struct VkSurfaceTransformFlagsKHR { ::VkFlags
        eIdentity:1,
        eRotate90:1,
        eRotate180:1,
        eRotate270:1,
        eHorizontalMirror:1,
        eHorizontalMirrorRotate90:1,
        eHorizontalMirrorRotate180:1,
        eHorizontalMirrorRotate270:1,
        eInherit:1;

        OPERATORS(VkSurfaceTransformFlagsKHR,::VkSurfaceTransformFlagBitsKHR,::VkFlags);
    };

    struct VkCompositeAlphaFlagsKHR { ::VkFlags
        eOpaque: 1,
        ePreMultiplied:1,
        ePostMultiplied:1,
        eInherit:1;

        OPERATORS(VkCompositeAlphaFlagsKHR,::VkCompositeAlphaFlagBitsKHR,::VkFlags);
    };

#pragma pack(push, 1)
    struct VkGeometryInstanceFlagsNV { uint8_t
        eTriangleCullDisable:1,
        eTriangleFrontCounterclockwise:1,
        eForceOpaque:1,
        eForceNoOpaque:1;
        
        OPERATORS(VkGeometryInstanceFlagsNV,::VkGeometryInstanceFlagBitsNV,::uint8_t);
    };
#pragma pack(pop)


};
