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

    class Instance : public std::enable_shared_from_this<Instance> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};

        public: 
            Instance(api::Instance* instance = nullptr, api::InstanceCreateInfo info = {}) : lastinst(instance) {
                *instance = api::createInstance(info);
            };
    };

    class PhysicalDeviceHelper_T : public std::enable_shared_from_this<PhysicalDeviceHelper_T> {
        protected:
            api::PhysicalDevice physicalDevice = {};
            api::PhysicalDeviceFeatures2 features = {};
            api::PhysicalDeviceProperties2 properties = {};
            std::vector<uint32_t> queueFamilyIndices = {};
            Allocator allocator = {};

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

            Allocator& getAllocator() { return allocator; };
            const Allocator& getAllocator() const { return allocator; };

            // require to generate both VMA and vendor name 
            PhysicalDeviceHelper_T(const api::PhysicalDevice& physicalDevice) : physicalDevice(physicalDevice) {
                this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            };

            // require vendor name 
            PhysicalDeviceHelper_T(const api::PhysicalDevice& physicalDevice, const Allocator& allocator) : physicalDevice(physicalDevice), allocator(allocator) {
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
    };

    class Device_T : public std::enable_shared_from_this<Device_T> {
        protected: 

            friend Allocator;
            api::DeviceCreateInfo dfc = {};
            api::PipelineCache pipelineCache = {};
            api::DescriptorPool* descriptorPool = nullptr;
            api::Device* device = nullptr;
            PhysicalDeviceHelper physicalHelper = {};
            Allocator allocator = {};

        public: 
            Device_T(const PhysicalDeviceHelper& physicalHelper = {}, api::Device* device = nullptr, api::DeviceCreateInfo dfc = {}) : device(device), dfc(dfc), physicalHelper(physicalHelper) {
                if (physicalHelper && device && !(*device)) { *device = ((api::PhysicalDevice&)(*physicalHelper)).createDevice(dfc); };};

            // Get original Vulkan link 
            inline api::PipelineCache& getPipelineCache() { return pipelineCache; };
            inline const api::PipelineCache& getPipelineCache() const { return pipelineCache; };
            inline api::Device& least() { return *device; };
            inline const api::Device& least() const { return *device; };
            operator api::Device&() { return *device; };
            operator const api::Device&() const { return *device; };

            // 
            inline Device&& initialize();
            inline Device&& linkDescriptorPool(api::DescriptorPool* pool = nullptr) { this->descriptorPool = pool; return shared_from_this(); };
            inline Device&& linkAllocator(const Allocator& allocator = {}) { this->allocator = allocator; return shared_from_this(); };
            inline Device&& linkPhysicalHelper(const PhysicalDeviceHelper& physicalHelper = {}) { this->physicalHelper = physicalHelper; return shared_from_this(); };
            inline Device&& link(api::Device* dev = nullptr) { device = dev; return shared_from_this(); };
            inline const PhysicalDeviceHelper& getHelper() const { return physicalHelper; };
    };
};
