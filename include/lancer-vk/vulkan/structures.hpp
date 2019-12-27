#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "./vulkan/enums.hpp"
#include "./vulkan/bitfields.hpp"

namespace vkh {
    
#ifdef USE_GLM
    using VkExtent3D = glm::uvec3;
#endif

    // 
    typedef struct VkBufferCreateInfo {
        VkStructureType     sType                       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        const void*         pNext                       = nullptr;
        VkBufferCreateFlags flags;
        VkDeviceSize        size                        = 4u;
        VkBufferUsageFlags  usage;
        VkSharingMode       sharingMode;                = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t            queueFamilyIndexCount       = 0u;
        const uint32_t*     pQueueFamilyIndices         = nullptr;
    } VkBufferCreateInfo;

    // 
    typedef struct VkImageCreateInfo {
        VkStructureType          sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        const void*              pNext                  = nullptr;
        VkImageCreateFlags       flags                  ;
        VkImageType              imageType              = VK_IMAGE_TYPE_1D;
        VkFormat                 format                 = VK_FORMAT_R8G8_UNORM;
        VkExtent3D               extent                 = {1u,1u};
        uint32_t                 mipLevels              = 1u;
        uint32_t                 arrayLayers            = 1u;
        VkSampleCountFlagBits    samples                = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling            tiling                 = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags        usage                  ;
        VkSharingMode            sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
        uint32_t                 queueFamilyIndexCount  = 0u;
        const uint32_t*          pQueueFamilyIndices    = nullptr;
        VkImageLayout            initialLayout          = VK_IMAGE_LAYOUT_UNDEFINED;
    } VkImageCreateInfo;

};
