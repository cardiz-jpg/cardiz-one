#!/bin/sh
set -eu
cmake -S . -B build -G Xcode -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build --config Release
echo "Build listo en build/CardizOne_artefacts/Release/"
