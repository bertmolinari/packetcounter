# packetcounter
Dependencies
- pthread
- curl (to install curl, I ran "apt-get install libcurl4-gnutls-dev" as referenced from here: https://askubuntu.com/questions/78183/installing-curl-h-library)
- azure rest c++ sdk (to install, I cloned the repo from github: https://github.com/Microsoft/cpprestsdk, and built the libraries using the instructions on the github page)
- azure storage c++ sdk (to install, I cloned the repo from github: https://github.com/Azure/azure-storage-cpp, and built the libraries using the instructions on the github page)
- boost (to install boost, I ran "sudo apt-get install libboost-all-dev" as refernced from here: https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu)

Notes
- When building the azure rest c++ sdk, I had to manually copy the build output up tp casablanca/Release/build.debug[release]/Binaries
