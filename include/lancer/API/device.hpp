#pragma once

#include "../lib/core.hpp"
//#include "../API/memory.hpp"

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

    class Instance_T : public std::enable_shared_from_this<Instance_T> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};
            api::InstanceCreateInfo cif = {};
            
        public: 
            Instance_T(api::Instance* instance = nullptr, const api::InstanceCreateInfo& info = {}) : lastinst(instance), cif(info) {
                //*instance = api::createInstance(info);
                lastinst = instance;
            };

            inline api::Instance& least() { return *lastinst; };
            inline const api::Instance& least() const { return *lastinst; };
            operator api::Instance&() { return *lastinst; };
            operator const api::Instance&() const { return *lastinst; };
    };

    class PhysicalDeviceHelper_T : public std::enable_shared_from_this<PhysicalDeviceHelper_T> {
        protected:
            api::PhysicalDevice physicalDevice = {};
            api::PhysicalDeviceFeatures2 features = {};
            api::PhysicalDeviceProperties2 properties = {};
            std::vector<uint32_t> queueFamilyIndices = {};
            MemoryAllocator allocator = {};

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
            friend lancer::DeviceMaker;

            MemoryAllocator& getAllocator() { return allocator; };
            const MemoryAllocator& getAllocator() const { return allocator; };

            // require to generate both VMA and vendor name 
            PhysicalDeviceHelper_T(const api::PhysicalDevice& physicalDevice) : physicalDevice(physicalDevice) {
                this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            };

            // require vendor name 
            PhysicalDeviceHelper_T(const api::PhysicalDevice& physicalDevice, const MemoryAllocator& allocator) : physicalDevice(physicalDevice), allocator(allocator) {
                this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            };

            // getter of vendor name 
            operator const std::shared_ptr<paths::DriverWrapBase>&() const { return driverWrap; };
            inline std::string getPath(const std::string fpath) const { return driverWrap->getPath(fpath); };
            inline std::string getDriverName() const { return driverWrap->getDriverName(); };

            // 
            inline uint32_t getRecommendedSubgroupSize() {
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
            api::PhysicalDevice& least() { return physicalDevice; };
            const api::PhysicalDevice& least() const { return physicalDevice; };

            // 
            inline DeviceMaker&& createDeviceMaker(const api::DeviceCreateInfo& info = {}, api::Device* device = nullptr);
    };

    class Device_T : public std::enable_shared_from_this<Device_T> {
        protected: 
            friend MemoryAllocator;
            api::DeviceCreateInfo dfc = {};
            api::PipelineCache pipelineCache = {};
            api::DescriptorPool* descriptorPool = nullptr;
            api::Device* device = nullptr;
            PhysicalDeviceHelper physicalHelper = {};
            MemoryAllocator allocator = {};

        public: 
            Device_T(const PhysicalDeviceHelper& physicalHelper = {}, api::DeviceCreateInfo dfc = {}, api::Device* device = nullptr) : device(device), dfc(dfc), physicalHelper(physicalHelper) {
                if (physicalHelper && device && !(*device)) { *device = physicalHelper->least().createDevice(dfc); };};

            // Get original Vulkan link 
            operator api::Device&() { return *device; };
            operator const api::Device&() const { return *device; };

            // 
            inline DeviceMaker&& initialize();
            inline DeviceMaker&& linkDescriptorPool(api::DescriptorPool* pool = nullptr) { this->descriptorPool = pool; return shared_from_this(); };
            inline DeviceMaker&& linkAllocator(const MemoryAllocator& allocator = {}) { this->allocator = allocator; return shared_from_this(); };
            inline DeviceMaker&& linkPhysicalHelper(const PhysicalDeviceHelper& physicalHelper = {}) { this->physicalHelper = physicalHelper; return shared_from_this(); };
            inline DeviceMaker&& link(api::Device* dev = nullptr) { device = dev; return shared_from_this(); };

            template<class T = MemoryAllocator_T> inline const auto& getAllocator() const { return std::dynamic_pointer_cast<T>(allocator); };
            template<class T = MemoryAllocator_T> inline auto& getAllocator() { return std::dynamic_pointer_cast<T>(allocator); };

            // 
            inline auto& least() { return *device; };
            inline const auto least() const { return *device; };
            inline auto& getHelper() { return this->physicalHelper; };
            inline const auto& getHelper() const { return this->physicalHelper; };
            inline auto& getDescriptorPool() { return *this->descriptorPool; };
            inline const auto& getDescriptorPool() const { return *this->descriptorPool; };
            inline auto& getPipelineCache() { return *this->pipelineCache; };
            inline const auto& getPipelineCache() const { return *this->pipelineCache; };

            // 
            inline BufferMaker&& createBufferMaker(const api::BufferCreateInfo& bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Buffer* lastbuf = nullptr);
            inline ImageMaker&& createImageMaker(const api::ImageCreateInfo& bfc = api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Image* lastbuf = nullptr);
            inline RenderPassMaker&& createRenderPassMaker(const api::RenderPassCreateInfo& bfc = api::RenderPassCreateInfo(), api::RenderPass* lastbuf = nullptr);
            inline GraphicsPipelineMaker&& createGraphicsPipelineMaker(const api::GraphicsPipelineCreateInfo& info = {}, api::Pipeline* pipeline = nullptr, const uint32_t& width = 1u, const uint32_t& height = 1u);
            inline DescriptorSetLayoutMaker&& createDescriptorSetLayoutMaker(const api::DescriptorSetLayoutCreateInfo& bfc = {}, api::DescriptorSetLayout* pipeline = nullptr);
            inline DescriptorSetMaker&& createDescriptorSet(const api::DescriptorSetAllocateInfo& info = {}, api::DescriptorSet* pipeline = nullptr);
            inline SBTHelper&& createSBTHelper(api::Pipeline* pipeline = nullptr);
            template<class T = MemoryAllocator_T> inline std::shared_ptr<T>&& createAllocator(const uintptr_t& info = uintptr_t(nullptr));
    };
};
