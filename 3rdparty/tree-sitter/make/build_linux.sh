set -e

rm -rf build-linux-debug/
mkdir build-linux-debug
cd build-linux-debug
cmake .. -DCMAKE_BUILD_TYPE:STRING="Debug" -DLLVM_USE_LINKER=mold && make -j12
cd ../

rm -rf build-linux-release/
mkdir build-linux-release
cd build-linux-release
cmake .. -DCMAKE_BUILD_TYPE:STRING="Release" -DLLVM_USE_LINKER=mold && make -j12

cd ../