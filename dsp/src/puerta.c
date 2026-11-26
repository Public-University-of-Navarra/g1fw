#include "puerta.h"

#include <stdio.h>

/* Umbrales: ajustar con la camara real. */
#define CR_ROJO 170       /* Cr alto = rojo */
#define Y_BRILLO 120      /* los LEDs encendidos son brillantes */
#define PASO 2            /* se mira 1 de cada 2 filas y columnas: 4 veces menos trabajo */
#define MIN_PIXELES 20    /* menos pixeles rojos que esto: no se ve la puerta */
#define ZONA_MUERTA 10    /* % del semiancho/semialto: dentro, se da por centrado */
#define DEMASIADO_CERCA 95 /* % del ancho que ocupa la puerta: mas, hay que retroceder */

static const char *const NOMBRES[] = {"quieto",  "arriba",   "abajo", "izquierda",
                                      "derecha", "adelante", "atras"};

puerta_cmd_t puerta_procesar(const puerta_imagen_t *img) {
  uint32_t x, y, n = 0;
  uint64_t sx = 0, sy = 0;
  uint32_t xmin = 0xFFFFFFFFu, xmax = 0;
  int32_t ex, ey; /* error del centro de la puerta respecto al de la imagen, en % */

  if (img == NULL || img->datos == NULL || img->ancho < 2 || img->alto < 2) return PUERTA_QUIETO;

  for (y = 0; y < img->alto; y += PASO) {
    const uint8_t *fila = img->datos + (size_t)y * img->ancho * 2;
    for (x = 0; x + 1 < img->ancho; x += PASO) {
      const uint8_t *par = fila + (x / 2) * 4; /* Cb Y0 Cr Y1 de los pixeles x y x+1 */
      if (par[2] >= CR_ROJO && par[1] >= Y_BRILLO) {
        n++;
        sx += x;
        sy += y;
        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
      }
    }
  }
  if (n < MIN_PIXELES) return PUERTA_QUIETO; /* no se ve la puerta */

  ex = (int32_t)((int64_t)(sx / n) * 200 / img->ancho) - 100; /* -100 izquierda .. 100 derecha */
  ey = 100 - (int32_t)((int64_t)(sy / n) * 200 / img->alto);  /* -100 abajo .. 100 arriba */

  if ((xmax - xmin) * 100 >= DEMASIADO_CERCA * img->ancho) return PUERTA_ATRAS;
  if (ex < -ZONA_MUERTA || ex > ZONA_MUERTA || ey < -ZONA_MUERTA || ey > ZONA_MUERTA) {
    /* se corrige primero el eje mas desviado */
    if ((ex < 0 ? -ex : ex) >= (ey < 0 ? -ey : ey))
      return ex < 0 ? PUERTA_IZQUIERDA : PUERTA_DERECHA;
    return ey > 0 ? PUERTA_ARRIBA : PUERTA_ABAJO;
  }
  return PUERTA_ADELANTE; /* centrada: a por ella */
}

size_t puerta_linea(char *buf, size_t tam, uint32_t t_ms, puerta_cmd_t cmd) {
  int n;
  if (buf == NULL || (unsigned)cmd > PUERTA_ATRAS) return 0;
  n = snprintf(buf, tam, "[dsp] t=%lu cmd=%s\n", (unsigned long)t_ms, NOMBRES[cmd]);
  return (n > 0 && (size_t)n < tam) ? (size_t)n : 0;
}
