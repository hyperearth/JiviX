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
    $ARGS = "$$Dir$Name.spv -Fo $Dir$Name.spv $AddArg OPTFLAGS"
    #$process = start-process -NoNewWindow -Filepath "spirv-opt" -ArgumentList "$ARGS" -PassThru
    #$process.PriorityClass = 'BelowNormal'
    $process.WaitForExit()
    $process.Close()
}

function BuildCompute($Name, $InDir = "", $OutDir = "", $AddArg = "", $AltName = $Name) {
    $ARGS = "$CMPPROF $InDir$Name.hlsl -Fo $OutDir$AltName.spv $CFLAGSV"
    $process = start-process -NoNewWindow -Filepath "dxc" -ArgumentList "$ARGS $AddArg" -PassThru
    #$ARGS = "$CMPPROF $InDir$Name -o $OutDir$AltName.spv $CFLAGSV"
    #$process = start-process -NoNewWindow -Filepath "glslangValidator" -ArgumentList "$ARGS" -PassThru
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
    
    BuildCompute "rasterize.ps"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    #BuildCompute "rasterize.gs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "rasterize.vs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    
    BuildCompute "rasterize.ps"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE" "covergence.ps"
    #BuildCompute "rasterize.gs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE" "covergence.gs"
    BuildCompute "rasterize.vs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE" "covergence.vs"

    # ray-tracing of vector graphics
    BuildCompute "denoise.cs"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"
    BuildCompute "denoise.cs"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5 -DLATE_STAGE" "reflect.cs" 
    BuildCompute "render.ps"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    BuildCompute "render.vs"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"

    # 
    BuildCompute "quad.cs"            "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"
    BuildCompute "resample.ps"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    BuildCompute "resample.gs"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "resample.vs"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    BuildCompute "transform.gs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
    BuildCompute "transform.vs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"

    #
    BuildCompute "mapping.ps"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE"
    BuildCompute "mapping.gs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE"
    BuildCompute "mapping.vs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE"

    # 
    BuildCompute "raytrace.rgs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
    BuildCompute "raytrace.chs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
    BuildCompute "raytrace.mhs"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
    BuildCompute "raytrace.cs"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"

    # optimize built shaders
    OptimizeMainline $RNDX
}
