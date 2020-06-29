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
    $ARGS = "$OPTFLAGS $Dir$Name.spv -Fo $Dir$Name.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "spirv-opt" -ArgumentList "$ARGS" -PassThru
    #$process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildCompute($Name, $InDir = "", $OutDir = "", $AddArg = "", $AltName = $Name) {
    $ARGS = "$CFLAGSV $CMPPROF $InDir$Name -Fo $OutDir$AltName.spv $AddArg"
    $process = start-process -NoNewWindow -Filepath "dxc" -ArgumentList "$ARGS" -PassThru
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
    BuildCompute "denoise.cs.hlsl"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"
    BuildCompute "denoise.cs.hlsl"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5 -DLATE_STAGE" "reflect.cs" 
    BuildCompute "render.ps.hlsl"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    BuildCompute "render.vs.hlsl"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    
    BuildCompute "rasterize.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    #BuildCompute "rasterize.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "rasterize.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    
    BuildCompute "rasterize.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE" "covergence.ps"
    #BuildCompute "rasterize.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE" "covergence.gs"
    BuildCompute "rasterize.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE" "covergence.vs"
    
    BuildCompute "mapping.ps.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE"
    BuildCompute "mapping.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE"
    BuildCompute "mapping.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE"
    
    BuildCompute "transform.gs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "transform.vs.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    BuildCompute "resample.ps.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    BuildCompute "resample.gs.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "resample.vs.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    BuildCompute "quad.cs.hlsl"            "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"

    # 
    BuildCompute "raytrace.cs.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rgen.hlsl"        "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rchit.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"
    BuildCompute "raytrace.rmiss.hlsl"       "$INDIR$RNDX" "$HRDDIR$RTPU"

    # optimize built shaders
    OptimizeMainline $RNDX
}
