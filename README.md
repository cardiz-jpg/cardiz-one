# CARDIZ ONE v0.6 PRE-LAUNCH DEMO

Plugin vocal/mastering para Ableton Live 12. La interfaz mantiene el diseño oficial negro carbón, marfil y dorado con cuatro controles, lectura momentánea estimada y modos ONE VOX / ONE MASTER / PRO.

## Análisis inteligente

- ONE VOX: perfiles LATINO URBANO, ELECTRONICA, BALADA y NATURAL.
- ONE MASTER: perfiles STREAMING MODERNO, CLUB / ELECTRONICA, BALADA / ORGANICO y TRANSPARENTE.
- PRO: estrategias IMPACTO CONTROLADO, DINAMICA ABIERTA, BALANCE CALIDO y REFERENCIA NEUTRA.
- El análisis escucha audio real, muestra progreso, informa RMS/balance tonal y aplica una propuesta audible.

## Diagnóstico y comparación v0.6

- Comparación ANTES / CARDIZ ONE igualada automáticamente por nivel RMS.
- Detección de sibilancia entre 5 y 10 kHz con reducción dinámica split-band.
- Detección de resonancias dominantes en cuatro zonas críticas y reducción selectiva.
- Informe visible de RMS, crest factor, balance tonal, sibilancia y frecuencia resonante.
- Recomendaciones explicadas según lo detectado y los cambios aplicados.
- Panel extendido con la misma estética oficial carbón, marfil y dorado.

## Motor actual

- Filtro subsónico y balance tonal/presencia.
- Compresión adaptada por GLUE y PUNCH.
- Saturación suave, control de anchura estéreo y limitador con techo configurable.
- Medidores de entrada/salida, lectura momentánea estimada y parámetros automatizables.
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

La lectura central se identifica como estimada: esta demo no sustituye una medición certificada BS.1770 ni un medidor true-peak con sobremuestreo.
