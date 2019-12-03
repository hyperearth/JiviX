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

    // TODO: Enumerable Devices 
    class Instance_T : public std::enable_shared_from_this<Instance_T> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};
            api::InstanceCreateInfo cif = {};
            
        public:
            Instance_T(const api::InstanceCreateInfo& info = {}, api::Instance* instance = nullptr) : lastinst(instance), cif(info) {
                if (lastinst) { *lastinst = api::createInstance(cif); };
            };
            inline InstanceMaker link(api::Instance* instance = nullptr) {  lastinst = instance; return shared_from_this(); };
            inline InstanceMaker create() { if (lastinst) { *lastinst = api::createInstance(cif); return shared_from_this(); }; };
            inline api::Instance& least() { return *lastinst; };
            inline const api::Instance& least() const { return *lastinst; };
            operator api::Instance&() { return *lastinst; };
            operator const api::Instance&() const { return *lastinst; };
    };

    class PhysicalDevice_T : public std::enable_shared_from_this<PhysicalDevice_T> {
        protected:
            InstanceMaker instance = {};
            api::PhysicalDevice physicalDevice = {};
            api::PhysicalDeviceFeatures2 features = {};
            api::PhysicalDeviceProperties2 properties = {};
            std::vector<uint32_t> queueFamilyIndices = {};
            //MemoryAllocator allocator = {};

            // required (if there is no, will generated)
            std::shared_ptr<paths::DriverWrapBase> driverWrap = {};

            inline virtual api::Result getVendorName(){
                driverWrap = paths::getNamedDriver(this->properties.properties.vendorID, this->features.features.shaderInt16);
                return api::Result::eSuccess;
            };

        public:
            friend lancer::DeviceMaker;

            //MemoryAllocator& getAllocator() { return allocator; };
            //const MemoryAllocator& getAllocator() const { return allocator; };

            // require to generate both VMA and vendor name 
            PhysicalDevice_T(const InstanceMaker& instance = {}, const api::PhysicalDevice& physicalDevice = {}) : physicalDevice(physicalDevice), instance(instance) {
                this->getFeaturesWithProperties(), this->getVendorName();
            };

            // Updated 03.12.2019
            inline virtual std::pair<api::PhysicalDeviceProperties2, api::PhysicalDeviceFeatures2> getFeaturesWithProperties(void* pNextProperties = nullptr, void* pNextFeatures = nullptr) {
                this->properties.pNext = pNextProperties;
                physicalDevice.getFeatures2(&this->features.setPNext(pNextFeatures));
                physicalDevice.getProperties2(&this->properties);
                return std::pair<api::PhysicalDeviceProperties2, api::PhysicalDeviceFeatures2>(this->properties, this->features);
            };

            // getter of vendor name 
            inline operator const std::shared_ptr<paths::DriverWrapBase>&() const { return driverWrap; };
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
            inline operator api::PhysicalDevice&() { return physicalDevice; };
            inline operator const api::PhysicalDevice&() const { return physicalDevice; };
            inline api::PhysicalDevice& least() { return physicalDevice; };
            inline const api::PhysicalDevice& least() const { return physicalDevice; };

            // 
            inline api::Instance& getInstance() { return instance->least(); };
            inline const api::Instance& getInstance() const { return instance->least(); };

            // 
            inline DeviceMaker createDeviceMaker(const api::DeviceCreateInfo& info = {}, api::Device* device = nullptr);
    };

    class Device_T : public std::enable_shared_from_this<Device_T> {
        protected: 
            friend MemoryAllocator;
            api::DispatchLoaderDynamic dispatcher = {};
            api::DeviceCreateInfo dfc = {};
            api::PipelineCache pipelineCache = {};
            api::DescriptorPool* descriptorPool = nullptr;
            api::Device* device = nullptr;
            PhysicalDeviceHelper physicalHelper = {};
            MemoryAllocator allocator = {};

        public: 
            Device_T(const PhysicalDeviceHelper& physicalHelper = {}, const api::DeviceCreateInfo& dfc = {}, api::Device* device = nullptr) : device(device), dfc(dfc), physicalHelper(physicalHelper) { 
//                if (physicalHelper && device && !(*device)) {
//                    *device = physicalHelper->least().createDevice(dfc);
//#ifdef VOLK_H_
//                    volkLoadDevice(*device);
//#endif
//                };
            };

            // Added Deferred Method for Create Device
            inline DeviceMaker create() {
                if (physicalHelper && device && !(*device)) {
                    *device = physicalHelper->least().createDevice(dfc);
                    dispatcher = vk::DispatchLoaderDynamic(physicalHelper->getInstance(), *device);
#ifdef VOLK_H_
                    volkLoadDevice(*device);
#endif
                };
                return shared_from_this();
            };

            template<class T = uint8_t>
            inline DeviceMaker cache(const std::vector<T>& data = {}) {
                pipelineCache = device->createPipelineCache(api::PipelineCacheCreateInfo().setInitialDataSize(data.size() * sizeof(T)).setPInitialData(data.data()));
                return shared_from_this();
            };

            // Get original Vulkan link 
            operator api::Device&() { return *device; };
            operator const api::Device&() const { return *device; };
            operator api::DispatchLoaderDynamic& () { return dispatcher; };
            operator const api::DispatchLoaderDynamic& () const { return dispatcher; };

            // 
            api::DispatchLoaderDynamic& getDispatcher() { return dispatcher; };
            const api::DispatchLoaderDynamic& getDispatcher() const { return dispatcher; };

            // 
            inline DeviceMaker initialize();
            inline DeviceMaker linkDescriptorPool(api::DescriptorPool* pool = nullptr) { this->descriptorPool = pool; return shared_from_this(); };
            inline DeviceMaker linkPhysicalHelper(const PhysicalDeviceHelper& physicalHelper = {}) { this->physicalHelper = physicalHelper; return shared_from_this(); };
            inline DeviceMaker linkAllocator(const MemoryAllocator& allocator = {}) { this->allocator = allocator; return shared_from_this(); };
            inline DeviceMaker link(api::Device* dev = nullptr) { device = dev; return shared_from_this(); };

            // Original Type
            inline const auto& getAllocatorPtr() const { return this->allocator; };
            inline auto& getAllocatorClearPtr() { return this->allocator; };

            // Converted Type
            template<class T = MemoryAllocator_T> inline const auto& getAllocator() const { return std::dynamic_pointer_cast<T>(this->allocator); };
            template<class T = MemoryAllocator_T> inline auto& getAllocator() { return std::dynamic_pointer_cast<T>(this->allocator); };

            // 
            inline auto& least() { return *device; };
            inline const auto least() const { return *device; };
            inline auto& getHelper() { return this->physicalHelper; };
            inline const auto& getHelper() const { return this->physicalHelper; };
            inline auto& getDescriptorPool() { return (api::DescriptorPool&)(*this->descriptorPool); };
            inline const auto& getDescriptorPool() const { return (api::DescriptorPool&)(*this->descriptorPool); };
            inline auto& getPipelineCache() { return this->pipelineCache; };
            inline const auto& getPipelineCache() const { return this->pipelineCache; };

            //
            inline PipelineLayoutMaker createPipelineLayoutMaker(const api::PipelineLayoutCreateInfo& info = {}, api::PipelineLayout* playout = nullptr);
            inline BufferMaker createBufferMaker(const api::BufferCreateInfo& bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Buffer* lastbuf = nullptr);
            inline SamplerMaker createSamplerMaker(const api::SamplerCreateInfo& sfc = {}, api::Sampler* lastsamp = nullptr);
            inline ImageMaker createImageMaker(const api::ImageCreateInfo& ifc = api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Image* lastimg = nullptr);
            inline RenderPassMaker createRenderPassMaker(const api::RenderPassCreateInfo& rpc = api::RenderPassCreateInfo(), api::RenderPass* lastbuf = nullptr);
            inline GraphicsPipelineMaker createGraphicsPipelineMaker(const api::GraphicsPipelineCreateInfo& info = {}, api::Pipeline* pipeline = nullptr, const uint32_t& width = 1u, const uint32_t& height = 1u);
            inline DescriptorSetLayoutMaker createDescriptorSetLayoutMaker(const api::DescriptorSetLayoutCreateInfo& bfc = {}, api::DescriptorSetLayout* dlayout = nullptr);
            inline DescriptorSetMaker createDescriptorSet(const api::DescriptorSetAllocateInfo& info = {}, api::DescriptorSet* descset = nullptr);
#ifdef EXTENSION_RTX
            inline SBTHelper createSBTHelper(const api::RayTracingPipelineCreateInfoNV& rpt = {}, api::Pipeline* pipeline = nullptr);
            inline InstancedAcceleration createInstancedAcceleration(const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr);
            inline GeometryAcceleration createGeometryAcceleration(const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr);
#endif
            template<class T = MemoryAllocator_T> inline MemoryAllocator& createAllocator(const uintptr_t& info = 0u);
    };
};
