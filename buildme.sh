#!/bin/bash
#gcc packetcounter.c -lpthread -lcurl -o packetcounter

STORAGE_INCLUDE_PATH=/mnt/c/Users/bertm/azure-storage-cpp/Microsoft.WindowsAzure.Storage/includes
STORAGE_LIBRARY_PATH=/mnt/c/Users/bertm/azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.debug/Binaries
CPPREST_INCLUDE_PATH=/mnt/c/Users/bertm/casablanca/Release/include
CPPREST_LIBRARY_PATH=/mnt/c/Users/bertm/casablanca/Release/build.debug/Binaries

# LD_LIBRARY_PATH=$CPPREST_LIBRARY_PATH


g++ -g packetcounter.cpp -std=c++11 -I$STORAGE_INCLUDE_PATH -I$CPPREST_INCLUDE_PATH -L$STORAGE_LIBRARY_PATH -L$CPPREST_LIBRARY_PATH -lpthread -lcurl -lssl -lcrypto -lboost_system -lcpprest -lazurestorage -o packetcounter

#INC=/mnt/c/Users/bertm/azure-storage-cpp/Microsoft.WindowsAzure.Storage/include
#g++ packetcounter.cpp -I$INC -lpthread -lcurl
