#pragma once

#include "./core.hpp"
#include "./driver.hpp"
#include "./JVI/context.hpp"
#include "./JVI/driver.hpp"

namespace jvx { 
    class Context : public Wrap<jvi::Context> { public: using T = jvi::Context;
        Context() {};
        Context(vkt::uni_ptr<jvi::Driver> driver) { this->object = std::make_shared<jvi::Context>(driver); };
        Context(vkt::uni_arg<jvx::Driver> driver) { this->object = std::make_shared<jvi::Context>(*driver); };
        Context(vkt::uni_ptr<jvi::Context> obj) { this->object = obj; };
        Context(jvx::Driver driver) { this->object = std::make_shared<jvi::Context>(driver); };
        Context(std::shared_ptr<jvi::Context> obj) { this->object = obj; };
        Context(std::shared_ptr<jvi::Driver> driver) { this->object = std::make_shared<jvi::Context>(driver); };

        CALLIFY(sharedPtr);
        CALLIFY(setThread);
        CALLIFY(getThread);
        CALLIFY(refScissor);
        CALLIFY(refViewport);
        CALLIFY(refRenderPass);
        CALLIFY(getDriver);
        CALLIFY(createRenderPass);
        CALLIFY(getDescriptorSets);
        CALLIFY(getPipelineLayout);
        CALLIFY(registerTime);
        CALLIFY(drawTime);
        CALLIFY(timeDiff);
        CALLIFY(setDrawCount);
        CALLIFY(setPerspective);
        CALLIFY(setModelView);
        CALLIFY(getFlip0Buffers);
        CALLIFY(getFlip1Buffers);
        CALLIFY(getFrameBuffers);
        CALLIFY(getFlip0Buffer);
        CALLIFY(getFlip1Buffer);
        CALLIFY(getFrameBuffer);
        CALLIFY(createFramebuffers);
        CALLIFY(createDescriptorSetLayouts);
        CALLIFY(createDescriptorSets);
        CALLIFY(initialize);
    };
};
