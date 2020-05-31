#!/usr/bin/pwsh-preview

# It is helper for compilation shaders to SPIR-V

$INDIR="./"
$OUTDIR="../prebuilt/shaders/$VNDR/"
$HRDDIR="../prebuilt/intrusive/$VNDR/"
$RNDX="rtrace/"
$RTPU="rtrace/"

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

function OptimizeMainline($Pfx = "", $RDXI="radix/") {
    # optimize radix sort
    
}

function BuildAllShaders($Pfx = "", $RDXI="radix/") {
    #[System.Threading.Thread]::CurrentThread.Priority = 'BelowNormal'
    #[System.Threading.Thread]::CurrentThread.Priority = 'Highest'

    new-item -Name $HRDDIR$RDXO -itemtype directory  -Force | Out-Null

    # ray-tracing of vector graphics
    BuildCompute "denoise.comp"         "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "denoise.comp"         "$INDIR$RNDX" "$HRDDIR$RNDX" "-DLATE_STAGE" "reflect.comp" 
    BuildCompute "render.frag"          "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "render.vert"          "$INDIR$RNDX" "$HRDDIR$RNDX"
    
    BuildCompute "rasterize.frag"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    #BuildCompute "rasterize.geom"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "rasterize.vert"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    
    BuildCompute "rasterize.frag"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE" "covergence.frag"
    #BuildCompute "rasterize.geom"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE" "covergence.geom"
    BuildCompute "rasterize.vert"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE" "covergence.vert"
    
    BuildCompute "mapping.frag"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE"
    #BuildCompute "mapping.geom"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE"
    BuildCompute "mapping.vert"       "$INDIR$RNDX" "$HRDDIR$RNDX" "-DCONSERVATIVE"
    
    BuildCompute "transform.geom"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "transform.vert"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "resample.frag"        "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "resample.geom"        "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "resample.vert"        "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "quad.comp"            "$INDIR$RTPU" "$HRDDIR$RTPU"

    # 
    BuildCompute "raytrace.comp"        "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "raytrace.rgen"        "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "raytrace.rchit"       "$INDIR$RNDX" "$HRDDIR$RNDX"
    BuildCompute "raytrace.rmiss"       "$INDIR$RNDX" "$HRDDIR$RNDX"

    # optimize built shaders
    OptimizeMainline $RTPU
}
