#!/snap/bin/pwsh-preview
# -Vd are REQUIRED!
$CFLAGSV="-fvk-use-dx-layout -Vi -Vd -Zpr -O3" #
#$CFLAGSV="--source-entrypoint main -e main --target-env spirv1.5 -V -d -t --aml --nsf"

$VNDR="hlsl"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
