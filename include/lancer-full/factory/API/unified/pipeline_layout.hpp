#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class pipeline_layout_t : public std::enable_shared_from_this<pipeline_layout_t> { public: 
                core::api::pipeline_layout_t layout_ = API_NULL_HANDLE;

                pipeline_layout_t() {};
                pipeline_layout_t(const pipeline_layout_t& pipeline_layout_t = {}) : layout_(pipeline_layout_t) {};
                pipeline_layout_t& operator=(const pipeline_layout_t& pipeline_layout_t) { layout_ = pipeline_layout_t; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(layout_); };
                operator const uintptr_t&() const { return (uintptr_t&)(layout_); };
                operator core::api::pipeline_layout_t&() { return layout_; };
                operator const core::api::pipeline_layout_t&() const { return layout_; };

                core::api::pipeline_layout_t* operator->() { return &(this->layout_); };
                const core::api::pipeline_layout_t* operator->() const { return &(this->layout_); };
                
            };
        };
    };
};
