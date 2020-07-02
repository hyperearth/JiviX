#ifndef DRIVER_HLSL
#define DRIVER_HLSL

#ifdef TRANSFORM_FEEDBACK
#include "./tf.hlsli"
#else
#include "./index.hlsli"
#endif

#endif
