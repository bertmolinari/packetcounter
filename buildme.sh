#!/bin/bash
gcc packetcounter.c -lpthread -lcurl -o packetcounter

g++ packetcounter.cpp -lpthread -lcurl -o packetcounter2
