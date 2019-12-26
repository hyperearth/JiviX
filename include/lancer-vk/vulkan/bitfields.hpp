#include <vulkan/vulkan.h>

namespace vkh {

#define OPERATORS(NAME) \
        operator uint32_t& () { return reinterpret_cast<uint32_t&>(*this); };                                                                                     \
        operator const uint32_t& () const { return reinterpret_cast<const uint32_t&>(*this); };                                                                   \
        NAME& operator=(const VkBufferCreateFlags& F) { (uint32_t&)(*this) = (uint32_t&)F; return *this; };                                        \
        NAME& operator=(const uint32_t& F) { (uint32_t&)(*this) = F; return *this; };                                                              \
        NAME operator|(const VkBufferCreateFlags& F) { auto f = uint32_t(F) | uint32_t(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator&(const VkBufferCreateFlags& F) { auto f = uint32_t(F) & uint32_t(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator^(const VkBufferCreateFlags& F) { auto f = uint32_t(F) ^ uint32_t(*this); return reinterpret_cast<NAME&>(f); };\
        NAME operator~() { auto f = ~uint32_t(*this); return reinterpret_cast<NAME&>(f); };


    struct VkBufferCreateFlags {
        uint32_t
            eSparseBinding : 1,
            eResidency : 1,
            eAlised : 1,
            eProtected : 1,
            eDeviceAddressCaptureReplay : 1;

        OPERATORS(VkBufferCreateFlags)
    };


    struct VkBufferUsageFlags {
        uint32_t
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

        OPERATORS(VkBufferUsageFlags)
    };
    

};
