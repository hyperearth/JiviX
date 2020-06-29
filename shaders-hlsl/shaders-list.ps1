#!/usr/bin/pwsh-preview

# It is helper for compilation shaders to SPIR-V

$INDIR="./"
$OUTDIR="../prebuilt/shaders/$VNDR/"
$HRDDIR="../prebuilt/intrusive/$VNDR/"
$RNDX="rtrace/"
$RTPU=""

$CMPPROF=""
$OPTFLAGS="-O --skip-validation --strip-debug --inline-entry-points-exhaustive --strength-reduction --vector-dce --workaround-1209 --replace-invalid-opcode --ccp --unify-const --simplify-instructions --remove-duplicates --combine-access-chains  --convert-local-access-chains --private-to-local --merge-return --merge-blocks --if-conversion --cfg-cleanup --flatten-decorations --freeze-spec-const "

function Pause ($Message = "Press any key to continue . . . ") {
#    if ((Test-Path variable:psISE) -and $psISE) {
#        $Shell = New-Object -ComObject "WScript.Shell"
#        $Button = $Shell.Popup("Click OK to continue.", 0, "Script Paused", 0)
#    }
#    else {     
#        Write-Host -NoNewline $Message
#        [void][System.Console]::ReadKey($true)
#        Write-Host
#    }
}

function Optimize($Name, $Dir = "", $AddArg = "") {
    $ARGS = "$OPTFLAGS $Dir$Name.spv -o $Dir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "spirv-opt" -ArgumentList "$ARGS" -PassThru
    #$process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildCompute($Name, $InDir = "", $OutDir = "", $AddArg = "", $AltName = $Name) {
    $ARGS = "$CFLAGSV $CMPPROF $InDir$Name -o $OutDir$AltName.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
    #$process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function OptimizeMainline($Pfx = "", $RNDX="radix/") {
    # optimize radix sort
}

function BuildAllShaders($Pfx = "") {
    #[System.Threading.Thread]::CurrentThread.Priority = 'BelowNormal'
    #[System.Threading.Thread]::CurrentThread.Priority = 'Highest'

    new-item -Name $HRDDIR$RDXO -itemtype directory  -Force | Out-Null

    # ray-tracing of vector graphics
    BuildCompute "denoise.cs.hlsl"         "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "denoise.cs.hlsl"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-DLATE_STAGE" "reflect.cs" 
    BuildCompute "render.ps.hlsl"          "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "render.vs.hlsl"          "$INDIR$RNDX" "$HRDDIR$RTPU"
    
    BuildCompute "rasterize.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    #BuildCompute "rasterize.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "rasterize.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    
    BuildCompute "rasterize.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE" "covergence.ps"
    #BuildCompute "rasterize.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE" "covergence.gs"
    BuildCompute "rasterize.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE" "covergence.vs"
    
    BuildCompute "mapping.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE"
    BuildCompute "mapping.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE"
    BuildCompute "mapping.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-DCONSERVATIVE"
    
    BuildCompute "transform.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "transform.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "resample.ps.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "resample.gs.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "resample.vs.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "quad.cs.hlsl"            "$INDIR$RNDX" "$HRDDIR$RTPU"

    # 
    BuildCompute "raytrace.comp"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rgen"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rchit"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rmiss"       "$INDIR$RNDX" "$HRDDIR$RTPU"

    # optimize built shaders
    OptimizeMainline $RNDX
}
