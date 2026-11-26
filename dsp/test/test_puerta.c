/* Prueba en el PC: gcc -I../src ../src/puerta.c test_puerta.c && ./a.out */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "puerta.h"

#define W 320
#define H 240
static uint8_t img[W * H * 2];

/* Fondo gris y un rectangulo rojo brillante de (x0,y0) a (x1,y1). */
static puerta_cmd_t probar(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
  puerta_imagen_t im = {img, W, H};
  uint32_t x, y;
  for (y = 0; y < H; y++)
    for (x = 0; x < W; x += 2) {
      uint8_t *p = &img[(y * W + x) * 2];
      int rojo = x >= x0 && x <= x1 && y >= y0 && y <= y1;
      p[0] = rojo ? 90 : 128;  /* Cb */
      p[1] = rojo ? 150 : 100; /* Y0 */
      p[2] = rojo ? 240 : 128; /* Cr */
      p[3] = p[1];             /* Y1 */
    }
  return puerta_procesar(&im);
}

int main(void) {
  char linea[32];
  assert(probar(1, 1, 0, 0) == PUERTA_QUIETO);         /* sin rojo */
  assert(probar(140, 100, 180, 140) == PUERTA_ADELANTE); /* centrada */
  assert(probar(10, 100, 50, 140) == PUERTA_IZQUIERDA);
  assert(probar(270, 100, 310, 140) == PUERTA_DERECHA);
  assert(probar(140, 5, 180, 40) == PUERTA_ARRIBA);
  assert(probar(140, 200, 180, 235) == PUERTA_ABAJO);
  assert(probar(0, 0, W - 1, H - 1) == PUERTA_ATRAS); /* ocupa toda la imagen */
  assert(puerta_procesar(NULL) == PUERTA_QUIETO);
  assert(puerta_linea(linea, sizeof(linea), 15320, PUERTA_IZQUIERDA) > 0);
  assert(strcmp(linea, "[dsp] t=15320 cmd=izquierda\n") == 0);
  assert(puerta_linea(linea, 8, 1, PUERTA_QUIETO) == 0); /* no cabe */
  puts("OK");
  return 0;
}
