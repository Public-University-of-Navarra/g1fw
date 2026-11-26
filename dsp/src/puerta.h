#ifndef PUERTA_H
#define PUERTA_H

#include <stddef.h>
#include <stdint.h>

/* Las 7 ordenes que la DSP manda al ESP32 (mismos codigos que camera_cmd_t). */
typedef enum {
  PUERTA_QUIETO = 0,
  PUERTA_ARRIBA = 1,
  PUERTA_ABAJO = 2,
  PUERTA_IZQUIERDA = 3,
  PUERTA_DERECHA = 4,
  PUERTA_ADELANTE = 5,
  PUERTA_ATRAS = 6
} puerta_cmd_t;

/* Imagen YCbCr 4:2:2 entrelazada (UYVY: Cb Y0 Cr Y1), la que da el decodificador de video
   de la LCDK. ancho y alto en pixeles; ancho par. */
typedef struct {
  const uint8_t *datos;
  uint32_t ancho;
  uint32_t alto;
} puerta_imagen_t;

/* Busca los LEDs rojos y decide la orden. */
puerta_cmd_t puerta_procesar(const puerta_imagen_t *img);

/* Escribe "[dsp] t=<ms> cmd=<orden>\n" en buf. Devuelve los bytes escritos, 0 si no cabe. */
size_t puerta_linea(char *buf, size_t tam, uint32_t t_ms, puerta_cmd_t cmd);

#endif
