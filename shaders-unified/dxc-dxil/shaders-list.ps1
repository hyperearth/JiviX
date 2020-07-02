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
    $ARGS = "$CMPPROF $InDir$Name.hlsl -Fo $OutDir$AltName.dxil $CFLAGSV"
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


    BuildCompute "rasterize.frag"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
    #BuildCompute "rasterize.geom"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
     BuildCompute "rasterize.vert"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
    
     BuildCompute "rasterize.frag"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE" "covergence.frag"
    #BuildCompute "rasterize.geom"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE" "covegeomence.gs"
     BuildCompute "rasterize.vert"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE" "covergence.vert"

    # ray-tracing of vector graphics
     BuildCompute "denoise.comp"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"
     BuildCompute "denoise.comp"         "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5 -DLATE_STAGE" "reflect.comp" 
     BuildCompute "render.frag"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
     BuildCompute "render.vert"          "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"

    # 
     BuildCompute "quad.comp"            "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"
     BuildCompute "resample.frag"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5"
     BuildCompute "resample.geom"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
     BuildCompute "resample.vert"        "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"
     BuildCompute "transform.geom"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5"
     BuildCompute "transform.vert"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5"

    #
     BuildCompute "mapping.frag"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T ps_6_5 -DCONSERVATIVE"
     BuildCompute "mapping.geom"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T gs_6_5 -DCONSERVATIVE"
     BuildCompute "mapping.vert"       "$INDIR$RNDX" "$HRDDIR$RTPU" "-T vs_6_5 -DCONSERVATIVE"

    # 
     BuildCompute "raytrace.rgen"      "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
     BuildCompute "raytrace.rchit"     "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
     BuildCompute "raytrace.rmiss"     "$INDIR$RNDX" "$HRDDIR$RTPU" "-T lib_6_5"
     BuildCompute "raytrace.comp"      "$INDIR$RNDX" "$HRDDIR$RTPU" "-T cs_6_5"

    # optimize built shaders
    OptimizeMainline $RNDX
}

