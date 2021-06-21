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

# .\bin\FINAL.exe 0 3 .\output\next_week.bmp

# .\bin\FINAL.exe 1 .\testcases\simpleLight.txt .\output\simpleLight.bmp

.\bin\FINAL.exe 1 .\testcases\weekend.txt .\output\weekend.bmp

# bin/FINAL testcases/scene01_basic.txt output/scene01.bmp
# bin/FINAL testcases/scene02_cube.txt output/scene02.bmp
# bin/FINAL testcases/scene03_sphere.txt output/scene03.bmp
# bin/FINAL testcases/scene04_axes.txt output/scene04.bmp
# bin/FINAL testcases/scene05_bunny_200.txt output/scene05.bmp
# bin/FINAL testcases/scene06_bunny_1k.txt output/scene06.bmp
# bin/FINAL testcases/scene07_shine.txt output/scene07.bmp