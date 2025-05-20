@echo off
echo Creating build directory...
if not exist build mkdir build
cd build

echo Generating build files...
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

echo Building project...
cmake --build . --config Debug

echo Running program...
.\Debug\wallet_system.exe

cd .. 