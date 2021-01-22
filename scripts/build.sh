rm -rf build-files
mkdir build-files
cd build-files/

# Build weak-isolation-mock-db
cmake ../ > log.txt

# Build applications
cmake --build ./ --target courseware_app shopping_cart_app twitter_app stack_app > app_log.txt
