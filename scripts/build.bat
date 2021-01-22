@echo off

set argC=0
for %%x in (%*) do Set /A argC+=1

if NOT %argC%==1 (
    echo invalid arguments
    echo please provide vcpkg cmake path
    EXIT /B 1
)

if EXIST build-files rmdir /Q /S build-files
mkdir build-files
cd build-files

echo Building weak-isolation-mock-db ...

cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=%1 ../ > log.txt

if errorlevel 1 (
    echo BUILD UNSUCCESSFUL
    echo See build-files/log.txt for details
    EXIT /B 1
)

echo Build Successful
echo Building Applications ...

cmake --build ./ --target courseware_app shopping_cart_app twitter_app stack_app

if errorlevel 1 (
    echo Applications BUILD UNSUCCESSFUL
    echo See build-files/app_log.txt for details
    EXIT /B 1
)
echo Build Successful
