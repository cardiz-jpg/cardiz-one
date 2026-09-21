@echo off
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
echo Build listo en build\CardizOne_artefacts\Release\
