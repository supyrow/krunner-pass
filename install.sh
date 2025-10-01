#!/bin/bash

set -e

rm -rf build
mkdir -p build
cd build


MAJOR_VERSION=$(krunner --version | sed 's/[^0-9.]*//g' | cut -d'.' -f1)

if [ "$MAJOR_VERSION" -eq 6 ]; then
    cmake .. -DQT_MAJOR_VERSION=6 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
elif [ "$MAJOR_VERSION" -eq 5 ]; then
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DCMAKE_BUILD_TYPE=Release
else
    echo "ERROR: Unsupported KRunner version ($MAJOR_VERSION). This script only supports versions 5 and 6." >&2
    exit 1
fi

make -j$(nproc)
sudo make install

if [ "$MAJOR_VERSION" -eq 6 ]; then
    kquitapp6 krunner || true
    krunner --replace &
elif [ "$MAJOR_VERSION" -eq 5 ]; then
    kquitapp5 krunner || true
    kstart5 krunner
fi
