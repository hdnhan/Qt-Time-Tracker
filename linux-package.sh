#!/bin/bash

# Configure
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/mQt -DCMAKE_BUILD_TYPE=Release

# Compile
cmake --build build --config Release --parallel

# Package
mkdir -p package/TimeTracker/DEBIAN
mkdir -p package/TimeTracker/usr/bin
mkdir -p package/TimeTracker/usr/share/applications
mkdir -p package/TimeTracker/usr/share/icons

cp build/TimeTracker package/TimeTracker/usr/bin
cp debian/control package/TimeTracker/DEBIAN
cp debian/TimeTracker.desktop package/TimeTracker/usr/share/applications
cp assets/timer.png package/TimeTracker/usr/share/icons

dpkg-deb --build package/TimeTracker
