#!/snap/bin/pwsh-preview

$CFLAGSV="--source-entrypoint main -e main --target-env spirv1.5 -V -d -t --aml --nsf"

$VNDR="hlsl"
. "./shaders-list.ps1"

BuildAllShaders ""

#pause for check compile errors
Pause
