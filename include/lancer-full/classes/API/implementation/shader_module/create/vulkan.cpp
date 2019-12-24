#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_pool.hpp"
#include "./factory/API/unified/shader_module.hpp"
#include "./classes/API/unified/shader_module.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<shader_module,core::api::result_t> shader_module::create(const shader_module_create_info& info) {
                vk::ShaderModuleCreateInfo vk_info = {};
                module_ = std::make_shared<api::factory::shader_module_t>((*device_)->createShaderModule(
                    vk::ShaderModuleCreateInfo().setPCode(info).setCodeSize(info)
                ));
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
