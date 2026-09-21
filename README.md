# CARDIZ ONE v0.3

Plugin vocal/mastering para Ableton Live 12. La interfaz mantiene el diseño oficial negro carbón, marfil y dorado con cuatro controles, medidor LUFS central y modos ONE VOX / ONE MASTER / PRO.

## Motor actual

- Filtro subsónico y balance tonal/presencia.
- Compresión adaptada por GLUE y PUNCH.
- Saturación suave, control de anchura estéreo y limitador true-peak configurable.
- Medidores de entrada/salida, lectura de loudness y parámetros automatizables.
- Estado recuperable dentro del proyecto de Ableton.

## Compilar en macOS Monterey (Intel)

1. Instala Xcode y CMake.
2. Ejecuta `sh scripts/build-mac.sh`.
3. Copia `CARDIZ ONE.vst3` a `~/Library/Audio/Plug-Ins/VST3/` y, opcionalmente, el AU a `~/Library/Audio/Plug-Ins/Components/`.
4. En Ableton Live 12: Preferencias > Plug-ins > activar VST3 y volver a escanear.

## Compilar en Windows

1. Instala Visual Studio 2022 con “Desktop development with C++”, Git y CMake.
2. Ejecuta `scripts\\build-windows.bat`.
3. Copia `CARDIZ ONE.vst3` a `C:\\Program Files\\Common Files\\VST3\\`.
4. Vuelve a escanear plugins en Ableton Live 12.

La primera compilación descarga JUCE 8.0.6. El AU solo se genera en macOS; Ableton admite VST3 en ambos sistemas.
