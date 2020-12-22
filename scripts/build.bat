mkdir -p build-files
cd build-files

cmake ../

cmake --build ./ --target kv_store_apps
