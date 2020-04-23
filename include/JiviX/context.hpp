#pragma once

#include "./core.hpp"
#include "./driver.hpp"
#include "./JVI/context.hpp"
#include "./JVI/driver.hpp"

namespace jvx { 
    class Context : public Wrap<jvi::Context> { public: using T = jvi::Context;
        Context(Driver driver) { object = std::make_shared<jvi::Context>(driver); };

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
        CALLIFY(createFramebuffers);
        CALLIFY(createDescriptorSetLayouts);
        CALLIFY(createDescriptorSets);
        CALLIFY(initialize);
    };
};
