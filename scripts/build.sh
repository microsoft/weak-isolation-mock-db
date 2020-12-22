mkdir -p build-files
cd build-files/

# Build weak-isolation-mock-db
cmake ../

# Build applications
cmake --build ./ --target kv_store_apps
