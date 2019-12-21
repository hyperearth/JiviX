#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class graphics_pipeline_t : public std::enable_shared_from_this<graphics_pipeline_t> { public: 
                core::api::graphics_pipeline_t pipeline_ = {};

                graphics_pipeline_t(const graphics_pipeline_t& graphics_pipeline_t) : pipeline_(graphics_pipeline_t) {};
                graphics_pipeline_t& operator=(const graphics_pipeline_t& graphics_pipeline_t) { pipeline_ = graphics_pipeline_t; return *this; };

                operator core::api::graphics_pipeline_t&() { return pipeline_; };
                operator const core::api::graphics_pipeline_t&() const { return pipeline_; };

                core::api::graphics_pipeline_t* operator->() { return &(this->pipeline_); };
                const core::api::graphics_pipeline_t* operator->() const { return &(this->pipeline_); };
                
            };
        };
    };
};
