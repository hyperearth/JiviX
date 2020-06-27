#extension GL_EXT_scalar_block_layout           : require
#extension GL_EXT_shader_realtime_clock         : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_nonuniform_qualifier          : require
#extension GL_EXT_control_flow_attributes       : require
#extension GL_ARB_shader_clock                  : require

#extension GL_EXT_shader_explicit_arithmetic_types         : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require
#extension GL_EXT_shader_subgroup_extended_types_int8      : require
#extension GL_EXT_shader_subgroup_extended_types_int16     : require
#extension GL_EXT_shader_subgroup_extended_types_int64     : require
#extension GL_EXT_shader_subgroup_extended_types_float16   : require
#extension GL_EXT_shader_16bit_storage                     : require
#extension GL_EXT_shader_8bit_storage                      : require
#extension GL_KHR_shader_subgroup_basic                    : require

#ifndef ENABLE_AS
#define ENABLE_AS
#endif

#ifdef ENABLE_AS
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#endif

precision highp float;
precision highp int;

#ifdef TRANSFORM_FEEDBACK
#include "./tf.glsl"
#else
#include "./index.glsl"
#endif
