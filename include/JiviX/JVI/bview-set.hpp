#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace jvi {

    // WIP Materials
    class BufferViewSet : public std::enable_shared_from_this<BufferViewSet> {
    public: friend Renderer;// 
        BufferViewSet() {};
        BufferViewSet(vkt::uni_ptr<Context> context) : context(context) { this->construct(); };
        //Material(Context* context) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
        ~BufferViewSet() {};

        // 
        virtual vkt::uni_ptr<BufferViewSet> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<BufferViewSet> sharedPtr() const { return std::shared_ptr<BufferViewSet>(shared_from_this()); };

        // 
        virtual uPTR(BufferViewSet) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(BufferViewSet) setContext(std::shared_ptr<Context> context) {
            this->context = context;
            return uTHIS;
        };

        // TODO: 
        virtual uPTR(BufferViewSet) createDescriptorSet() {
            return uTHIS;
        };

        // 
        virtual uPTR(BufferViewSet) resetBufferViews() {
            this->bufferViews.resize(0ull);
            return uTHIS;
        };

        // 
        virtual uintptr_t pushBufferView(const vkt::Vector<uint8_t>& bufferView) {
            auto ptr = this->bufferViews.size();
            this->bufferViews.push_back(bufferView); 
            return ptr;
        };

        // 
        virtual vkt::Vector<uint8_t>& get(const uint32_t& I = 0u) {
            return bufferViews[I];
        };

        // 
        virtual const vkt::Vector<uint8_t>& get(const uint32_t& I = 0u) const {
            return bufferViews[I];
        };

        // 
        virtual std::vector<vkt::Vector<uint8_t>>& getBufferViewList() {
            return bufferViews;
        };

        // 
        virtual const std::vector<vkt::Vector<uint8_t>>& getBufferViewList() const {
            return bufferViews;
        };

    protected: // 
        std::vector<vkt::Vector<uint8_t>> bufferViews = {};

        // TODO: 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vk::DescriptorSet descriptorSet = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };
};
