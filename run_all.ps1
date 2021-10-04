# If project not ready, generate cmake file.
$BUILD_EXISTS = (Test-Path ".\build")
if (!$BUILD_EXISTS){
    mkdir -Force build
    Set-Location build
    cmake -G "MinGW Makefiles" ..
    Set-Location ..
} 
else{
    Remove-Item -r build
    mkdir -Force build
    Set-Location build
    cmake -G "MinGW Makefiles" ..
    Set-Location ..
}

# Build project.
Set-Location build
make -j
Set-Location ..

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.
$OUTPUT_EXISTS = (Test-Path ".\output")
if (!$OUTPUT_EXISTS){
    mkdir -Force output
}

# .\bin\FINAL.exe 0 5 .\output\final_scene1.bmp
# .\bin\FINAL.exe 0 6 .\output\final_scene2.bmp
# .\bin\FINAL.exe 0 7 .\output\final_scene3.bmp

# .\bin\FINAL.exe 0 1 .\output\cornell_box_depth.bmp

.\bin\FINAL.exe 1 .\testcases\simpleLight.txt .\output\simpleLight.bmp
# .\bin\FINAL.exe 1 .\testcases\weekend.txt .\output\weekend.bmp
