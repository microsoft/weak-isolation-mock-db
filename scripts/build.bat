mkdir -p build-files
cd build-files

cmake ../  "-DCMAKE_TOOLCHAIN_FILE=%1"

cmake --build ./ --target kv_store_apps
