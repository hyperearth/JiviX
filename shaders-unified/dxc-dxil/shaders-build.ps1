#!/snap/bin/pwsh-preview
# -Vd are REQUIRED!
$CFLAGSV="-Vi -Vd -Zpr -O0" #
#$CFLAGSV="--source-entrypoint main -e main --target-env spirv1.5 -V -d -t --aml --nsf"

$VNDR="hlsl"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
