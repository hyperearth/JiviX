#!/snap/bin/pwsh-preview
# -Vd are REQUIRED!
$CFLAGSV="-fvk-use-scalar-layout -fspv-target-env=vulkan1.2 -spirv -Vi -Vd -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_AMD_shader_explicit_vertex_parameter "
#$CFLAGSV="--source-entrypoint main -e main --target-env spirv1.5 -V -d -t --aml --nsf"

$VNDR="hlsl"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
