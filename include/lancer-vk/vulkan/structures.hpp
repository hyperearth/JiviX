#include <vulkan/vulkan.h>

#include "./vulkan/enums.hpp"
#include "./vulkan/bitfields.hpp"

namespace vkh {
    
    struct VkBufferCreateInfo {
        VkStructureType     sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        const void*         pNext                    = nullptr;
        VkBufferCreateFlags flags;
        VkDeviceSize        size                     = 4u;
        VkBufferUsageFlags  usage;
        VkSharingMode       sharingMode;
        uint32_t            queueFamilyIndexCount;
        const uint32_t*     pQueueFamilyIndices;
    };

};
