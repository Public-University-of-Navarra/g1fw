# Firmware de la DSP (LCDK C6748)

Procesado de imagen. Por cada imagen:

1. Pulso en GPIO8[12] (DSP_SYNC) al empezar la imagen.
2. Al terminar el procesado, una linea por UART1 a 115200 baudios: `[dsp] n=<imagen> puerta=<0|1> dx=<-100..100> dy=<-100..100> dz=<-100..100>` (detalle en la seccion de la camara de `fw/README.md`).

En la LCDK hay que puentear R206 y R209.
