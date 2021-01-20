if EXIST build-files rmdir /Q /S build-files
mkdir build-files
cd build-files

cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=%1 ../

cmake --build ./ --target kv_store_apps
