#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace jvi {

    // WIP Materials
    class BufferViewSet : public std::enable_shared_from_this<BufferViewSet> { friend MeshInput;
    public: friend Renderer;// 
        BufferViewSet() {};
        BufferViewSet(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        BufferViewSet(const std::shared_ptr<Context>& context) : context(context) { this->construct(); };
        ~BufferViewSet() {};

        // 
        public: virtual vkt::uni_ptr<BufferViewSet> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<BufferViewSet> sharedPtr() const { return std::shared_ptr<BufferViewSet>(shared_from_this()); };

        // 
        protected: virtual uPTR(BufferViewSet) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            auto templ = vkh::VkDescriptorSetLayoutCreateInfo{}.also([&](auto* it) { vkt::zero32(it->flags); return it; });
            auto stagef = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 };
            auto incomp = vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1 };

            // 
            this->bufferViewSetLayoutHelper = templ;
            this->bufferViewSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 256u, .stageFlags = stagef }, incomp);
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->bufferViewSetLayoutHelper, nullptr, &this->bufferViewSetLayout[0]));

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(BufferViewSet) setContext(const vkt::uni_ptr<Context>& context) {
            this->context = context;
            return uTHIS;
        };

        // 
        public: virtual uPTR(BufferViewSet) resetBufferViews() {
            this->bufferViews.resize(0ull);
            return uTHIS;
        };

        // 
        public: virtual uintptr_t pushBufferView(const vkt::Vector<uint8_t>& bufferView) {
            auto ptr = this->bufferViews.size();
            this->bufferViews.push_back(bufferView); 
            return ptr;
        };

        // 
        public: virtual vkt::Vector<uint8_t>& get(const uint32_t& I = 0u) {
            return bufferViews[I];
        };

        // 
        public: virtual const vkt::Vector<uint8_t>& get(const uint32_t& I = 0u) const {
            return bufferViews[I];
        };

        // 
        public: virtual std::vector<vkt::Vector<uint8_t>>& getBufferViewList() {
            return bufferViews;
        };

        // 
        public: virtual const std::vector<vkt::Vector<uint8_t>>& getBufferViewList() const {
            return bufferViews;
        };

        // 
        public: virtual size_t getBufferCount() const { return bufferViews.size(); };
        public: virtual size_t getBufferCount() { return bufferViews.size(); };

        // 
        protected: virtual uPTR(BufferViewSet) createDescriptorSet() { //
            this->bufferViewSetHelper = vkh::VsDescriptorSetCreateInfoHelper(this->bufferViewSetLayout[0], this->driver->getDescriptorPool());

            // 
            for (uint32_t j = 0; j < this->bufferViews.size(); j++) {
                auto& handle = this->bufferViewSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .dstArrayElement = j,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                }).offset<VkBufferView>(0u) = this->bufferViews[j].createBufferView(VK_FORMAT_R8_UINT);
            };

            // 
            vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), this->bufferViewSetHelper, this->bufferViewSet[0], this->descriptorUpdated));

            // 
            return uTHIS;
        };

        // 
        public: virtual VkDescriptorSet& getDescriptorSet() {
            if (!this->bufferViewSet[0]) { this->createDescriptorSet(); };
            return this->bufferViewSet[0];
        };

        // 
        public: virtual const VkDescriptorSet& getDescriptorSet() const {
            return this->bufferViewSet[0];
        };

        // 
        public: virtual VkDescriptorSetLayout& getDescriptorLayout() {
            return bufferViewSetLayout[0];
        };

        // 
        public: virtual const VkDescriptorSetLayout& getDescriptorLayout() const {
            return bufferViewSetLayout[0];
        };


    protected: // 
        std::vector<vkt::Vector<uint8_t>> bufferViews = {};
        bool descriptorUpdated = false;

        // 
        std::vector<VkDescriptorSet> bufferViewSet = { VK_NULL_HANDLE };
        std::vector<VkDescriptorSetLayout> bufferViewSetLayout = { VK_NULL_HANDLE };
        vkh::VsDescriptorSetLayoutCreateInfoHelper bufferViewSetLayoutHelper = {};
        vkh::VsDescriptorSetCreateInfoHelper bufferViewSetHelper = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };
};
