set -e

rm -rf build-linux/
mkdir build-linux
cd build-linux
cmake .. -DCMAKE_BUILD_TYPE:STRING="Release" -DLLVM_USE_LINKER=mold && make -j12

cd ../