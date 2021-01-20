if EXIST build-files rmdir /Q /S build-files
mkdir build-files
cd build-files

cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=%1 ../ > log.txt

if errorlevel 1 EXIT /B 1

cmake --build ./ --target kv_store_apps > app_log.txt

if errorlevel 1 EXIT /B 1

ECHO BUILD SUCCESSFUL
