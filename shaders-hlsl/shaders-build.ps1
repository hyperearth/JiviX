#!/snap/bin/pwsh-preview

$CFLAGSV="-fvk-use-dx-layout -fspv-target-env=vulkan1.2"

$VNDR="hlsl"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
