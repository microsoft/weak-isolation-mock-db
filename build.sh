# Check if cpprestsdk already installed
if ! dpkg-query -W -f='${Status}' libcpprest-dev  | grep "ok installed";
then
    echo "cpprestsdk not found...installing..."
    # install cpprestsdk (Ubuntu)
    sudo apt-get install libcpprest-dev -y
fi

mkdir -p build-files
cd build-files/

# Build weak-isolation-mock-db
cmake ../

# Build applications
cmake --build ./ --target kv_store_apps
