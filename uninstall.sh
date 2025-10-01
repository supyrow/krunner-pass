#!/bin/bash

set -e

if [ ! -d "build" ]; then
    echo "ERROR: 'build' directory not found. Cannot run 'make uninstall'." >&2
    exit 1
fi

cd build
sudo make uninstall

MAJOR_VERSION=$(krunner --version | sed 's/[^0-9.]*//g' | cut -d'.' -f1)

if [ "$MAJOR_VERSION" -eq 6 ]; then
    kquitapp6 krunner || true
    krunner --replace &
elif [ "$MAJOR_VERSION" -eq 5 ]; then
    kquitapp5 krunner || true
    kstart5 krunner
fi
