#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class render_pass_t : public std::enable_shared_from_this<render_pass_t> { public: 
                core::api::render_pass_t render_pass_ = API_NULL_HANDLE;
                //core::api::pipeline_cache_t pipeline_cache_ = API_NULL_HANDLE;
                //std::vector<uint32_t> queue_family_indices_ = {};

                render_pass_t() {};
                render_pass_t(const render_pass_t& render_pass) : render_pass_(render_pass) {};
                render_pass_t(const core::api::render_pass_t& render_pass_) : render_pass_(render_pass_) {};
                render_pass_t& operator=(const render_pass_t& render_pass) { this->render_pass_ = render_pass; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(render_pass_); };
                operator const uintptr_t&() const { return (uintptr_t&)(render_pass_); };
                operator core::api::render_pass_t&() { return render_pass_; };
                operator const core::api::render_pass_t&() const { return render_pass_; };

                core::api::render_pass_t* operator->() { return &(this->render_pass_); };
                const core::api::render_pass_t* operator->() const { return &(this->render_pass_); };
                core::api::render_pass_t& operator*() { return (this->render_pass_); };
                const core::api::render_pass_t& operator*() const { return (this->render_pass_); };
            };

        };
    };
};
