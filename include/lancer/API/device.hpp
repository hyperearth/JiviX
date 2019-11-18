#pragma once

#include "../lib/core.hpp"
#include "../lib/memory.hpp"

namespace lancer {
    
    namespace paths {
        static inline const std::array<std::string, 6> pathNames{ "universal", "amdgpu", "nvidia", "intel", "vega10", "turing" };

        static inline constexpr const auto permutation = "radix/permutation.comp";
        static inline constexpr const auto partition = "radix/partition.comp";
        static inline constexpr const auto counting = "radix/counting.comp";
        static inline constexpr const auto scattering = "radix/scattering.comp";
        static inline constexpr const auto indiction = "radix/indiction.comp";

        class Generation {
        protected:
            std::string mod = "";
        public:
            Generation(const std::string& mod = ""): mod(mod) {}
            operator const std::string&() const {
                return mod;
            }
        };


        class NVLegacy : public Generation { public: NVLegacy(const std::string& mod = "nvidia") { this->mod = mod; }; };
        class NVTuring : public Generation { public: NVTuring(const std::string& mod = "turing") { this->mod = mod; }; };

        class AMDLegacy : public Generation { public: AMDLegacy(const std::string& mod = "amdgpu") { this->mod = mod; }; };
        class AMDVega10 : public Generation { public: AMDVega10(const std::string& mod = "vega10") { this->mod = mod; }; };


        class UniversalType {
        protected:
            std::string pathname = "universal";
        public:
            UniversalType(const std::string& pathname = "universal") : pathname(pathname) {};
            operator const std::string&() const { return pathname; };
        };

        template<class M = NVLegacy>
        class NVIDIA : public UniversalType {
        public:
            NVIDIA(const std::string& pathname = "") { this->pathname = pathname + (const std::string&)M(); };
        };

        template<class M = AMDLegacy>
        class AMD : public UniversalType {
        public:
            AMD(const std::string& pathname = "") { this->pathname = pathname + (const std::string&)M(); };
        };

        class Intel : public UniversalType {
        public:
            Intel(const std::string& pathname = "intel") { this->pathname = pathname; };
        };



        class DriverWrapBase {
        protected: 
            std::shared_ptr<UniversalType> driverType = {};
            std::string directory = "";
        public:
            DriverWrapBase(const std::string& directory = "./intrusive") : directory(directory) {};
            std::string getPath(const std::string& fpath) const {
                return (directory + "/" + std::string(*driverType) + "/" + fpath + ".spv");
            }
            std::string getDriverName() const {
                return std::string(*driverType);
            }
        };

        template<class T = UniversalType>
        class DriverWrap: public DriverWrapBase {
        public:
            DriverWrap(const std::string& directory = "./intrusive") {
                this->directory = directory;
                this->driverType = std::static_pointer_cast<UniversalType>(std::make_shared<T>());
            }
        };

        inline std::shared_ptr<DriverWrapBase> getNamedDriver(const uint32_t& vendorID, const int modifier = 0, const std::string& directory = "./intrusive") {
            switch (vendorID) {
            case 4318:
                if (modifier) {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<NVIDIA<NVTuring>>>(directory));
                }
                else {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<NVIDIA<NVLegacy>>>(directory));
                }
                break;
            case 4098:
                if (modifier) {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<AMD<AMDVega10>>>(directory));
                }
                else {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<AMD<AMDLegacy>>>(directory));
                }
                break;
            case 32902:
                return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<Intel>>(directory));
                break;
            default:
                return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<UniversalType>>(directory));
            };
        };
    };




    class Device;

    class Instance : public std::enable_shared_from_this<Instance> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};

        public: 
            Instance(api::Instance* instance = nullptr, api::InstanceCreateInfo info = {}) : lastinst(instance) {
            };
    };

    class PhysicalDeviceHelper : public std::enable_shared_from_this<PhysicalDeviceHelper> {
        protected:
            api::PhysicalDevice physicalDevice = {};
            api::PhysicalDeviceFeatures2 features = {};
            api::PhysicalDeviceProperties2 properties = {};
            std::vector<uint32_t> queueFamilyIndices = {};
            std::shared_ptr<Allocator> allocator = {};

            // required (if there is no, will generated)
            std::shared_ptr<paths::DriverWrapBase> driverWrap = {};
            
            virtual api::Result getFeaturesWithProperties(){
                this->features = physicalDevice.getFeatures2();
                this->properties = physicalDevice.getProperties2();
                return api::Result::eSuccess;
            };

            virtual api::Result getVendorName(){
                driverWrap = paths::getNamedDriver(this->properties.properties.vendorID, this->features.features.shaderInt16);
                return api::Result::eSuccess;
            };

        public:
            friend lancer::Device;

            // require to generate both VMA and vendor name 
            PhysicalDeviceHelper(const api::PhysicalDevice& physicalDevice) : physicalDevice(physicalDevice) {
                this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            };

            // require vendor name 
            PhysicalDeviceHelper(const api::PhysicalDevice& physicalDevice, const std::shared_ptr<Allocator>& allocator) : physicalDevice(physicalDevice), allocator(allocator) {
                this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            };

            // getter of vendor name 
            operator const std::shared_ptr<paths::DriverWrapBase>&() const { return driverWrap; };
            std::string getPath(const std::string fpath) const { return driverWrap->getPath(fpath); };
            std::string getDriverName() const { return driverWrap->getDriverName(); };

            // 
            uint32_t getRecommendedSubgroupSize() {
                if (driverWrap->getDriverName() == "turing") { return 16u; }; // SM7!
                if (driverWrap->getDriverName() == "amdvlk") { return 16u; }; // GCN!
                if (driverWrap->getDriverName() == "vega10") { return 16u; }; // GCN!
                if (driverWrap->getDriverName() == "rdna10") { return 32u; }; // RDNA
                if (driverWrap->getDriverName() == "nvidia") { return 32u; }; // SM6?
                return 32u;
            };

            // api::PhysicalDevice caster
            operator api::PhysicalDevice&() { return physicalDevice; };
            operator const api::PhysicalDevice&() const { return physicalDevice; };
    };

    class Device : public std::enable_shared_from_this<Device> {
        protected: 
            api::PipelineCache pipelineCache = {};
            api::DescriptorPool descriptorPool = {};
            api::DeviceCreateInfo div = {};
            api::Device* device = nullptr;
            api::DescriptorPool *dscp = nullptr;
            std::shared_ptr<PhysicalDeviceHelper> physicalHelper;
            std::shared_ptr<Allocator> allocator = {};

        public: 
            Device(const std::shared_ptr<Instance>& instance) {
            };

            // Get original Vulkan link 
            api::Device& Least() { return *device; };
            operator api::Device&() { return *device; };
            const api::Device& Least() const { return *device; };
            operator const api::Device&() const { return *device; };

            // 
            std::shared_ptr<Device>&& Allocator(const std::shared_ptr<Allocator>& allocator);
            std::shared_ptr<Device>&& Initialize(const std::shared_ptr<lancer::PhysicalDeviceHelper>& physicalHelper);
            std::shared_ptr<Device>&& Link(api::Device& dev) { device = &dev; return shared_from_this(); };
            std::shared_ptr<Device>&& LinkDescriptorPool(api::DescriptorPool& pool) { dscp = &pool; return shared_from_this(); };
            const std::shared_ptr<PhysicalDeviceHelper>& GetHelper() const { return physicalHelper; };
    };

    // 
    std::shared_ptr<Device>&& Device::Initialize(const std::shared_ptr<lancer::PhysicalDeviceHelper>& physicalHelper) {
         this->physicalHelper = physicalHelper;
        *this->device = device;

        // get VMA allocator for device
        if (!this->allocator && this->physicalHelper->allocator) { this->allocator = this->physicalHelper->allocator; };
        if ( this->allocator) { Allocator->Initialize(shared_from_this()); };

        // descriptor pool
        if (!this->descriptorPool)
        {
            // pool sizes, and create descriptor pool
            std::vector<api::DescriptorPoolSize> psizes = { };
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eStorageBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eStorageTexelBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eInlineUniformBlockEXT).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eUniformBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eAccelerationStructureNV).setDescriptorCount(128));

            api::DescriptorPoolInlineUniformBlockCreateInfoEXT inlineDescPool{};
            inlineDescPool.maxInlineUniformBlockBindings = 2;
            this->descriptorPool = api::Device(*this).createDescriptorPool(api::DescriptorPoolCreateInfo().setPNext(&inlineDescPool).setPPoolSizes(psizes.data()).setPoolSizeCount(psizes.size()).setMaxSets(256).setFlags(api::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | api::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT));
        };

        // pipeline cache 
        if (!this->pipelineCache) { this->pipelineCache = api::Device(*this).createPipelineCache(api::PipelineCacheCreateInfo()); };

        // 
        return shared_from_this();
    };


};
