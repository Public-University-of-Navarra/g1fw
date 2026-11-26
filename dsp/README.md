# Firmware de la DSP (LCDK C6748)

Procesado de imagen. Por cada imagen:

1. Pulso en GPIO8[12] (DSP_SYNC) al empezar la imagen.
2. Al terminar el procesado, una linea por UART1 a 115200 baudios: `[dsp] n=<imagen> <clave>=<valor> ...` (ver la seccion de la camara en `fw/README.md`).

En la LCDK hay que puentear R206 y R209.
