# move up one directory so we're in the root where packet counter was cloned to
cd ..

# install packages that we need for building
sudo apt-get install g++ libboost-all-dev libwebsocketpp-dev openssl libssl-dev ninja-build libxml2-dev uuid-dev cmake

# clone the code for the Azure Storage C++ SDK and the Microsoft C++ REST SDK locally (we depend on storage, which in turn depends on the REST SDK)
git clone https://github.com/Azure/azure-storage-cpp.git
git clone https://github.com/Microsoft/cpprestsdk.git casablanca

# build the Microsoft C++ REST SDK, this info is from: https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux
cd casablanca
mkdir build.debug
cd build.debug
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug
ninja

# build the Azure Storage C++ SDK, this info is from: https://github.com/Azure/azure-storage-cpp
cd ../azure-storage-cpp/Microsoft.WindowsAzure.Storage
mkdir build.debug
cd build.debug
CASABLANCA_DIR=~/casablanca/build.debug CXX=g++ cmake -DCMAKE_BUILD_TYPE=Debug ..
make


