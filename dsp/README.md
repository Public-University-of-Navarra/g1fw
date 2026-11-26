# Firmware de la DSP (LCDK C6748)

Procesado de imagen: busca la puerta de LEDs rojos y, por cada imagen, manda una linea por UART1 a 115200 baudios:

```
[dsp] t=<ms> cmd=<quieto|arriba|abajo|izquierda|derecha|adelante|atras>
```

Detalle de cada orden en la seccion de la camara de `fw/README.md`. En la LCDK hay que puentear R206 y R209.
