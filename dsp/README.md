# Firmware de la DSP (LCDK C6748)

Procesado de imagen en C: busca la puerta de LEDs rojos y, por cada imagen, manda una linea por UART1 a 115200 baudios:

```
[dsp] t=<ms> cmd=<quieto|arriba|abajo|izquierda|derecha|adelante|atras>
```

Detalle de cada orden en la seccion de la camara de `fw/README.md`. En la LCDK hay que puentear R206 y R209.

## Codigo

- `src/puerta.c` y `src/puerta.h`: C portable, sin nada de la placa. No depende de la resolucion: el ancho y el alto se pasan con cada imagen.
  - `puerta_procesar(&img)`: busca los pixeles rojos y brillantes (Cr y Y altos en YCbCr 4:2:2, el formato del decodificador de video de la LCDK) mirando 1 de cada 2 filas y columnas, calcula su centro y decide la orden:
    1. menos de 20 pixeles rojos: `quieto` (no se ve la puerta);
    2. la puerta ocupa casi todo el ancho: `atras`;
    3. el centro se sale de la zona muerta (10 %): se corrige el eje mas desviado (`izquierda`/`derecha` o `arriba`/`abajo`);
    4. centrada: `adelante`.
  - `puerta_linea(buf, tam, t_ms, cmd)`: prepara la linea para la UART.
- `test/test_puerta.c`: prueba en el PC con imagenes sinteticas: `cd dsp/test && gcc -I../src ../src/puerta.c test_puerta.c && ./a.out`.

Los umbrales estan al inicio de `puerta.c` y hay que ajustarlos con la camara real.

## Pendiente (en el proyecto de Code Composer Studio)

Falta la parte que depende de la placa: capturar cada imagen y enviar la linea. El bucle principal queda asi:

```c
for (;;) {
  esperar_imagen(&img);  /* captura del decodificador de video */
  n = puerta_linea(linea, sizeof(linea), milisegundos(), puerta_procesar(&img));
  uart1_enviar(linea, n);
}
```
