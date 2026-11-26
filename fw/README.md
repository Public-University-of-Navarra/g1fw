# Firmware FPF-LAB
## Device

El microcontrolador seleccionado es el ESP32 DevKit M1. En `platformio.ini` se debe utilizar el entorno `esp32`, que es el que compila CI mediante `pio run -e esp32`:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    paulstoffregen/OneWire
    milesburton/DallasTemperature
```

## Componentes

### IMU (imu, ICM-40609-D)

Se intenta montar el chip en la placa (U502); si no se puede, se usa la placa de desarrollo en J501. Los dos van al mismo canal y dirección, así que el código es el mismo.

ICM-40609-D de TDK InvenSense: acelerómetro y giróscopo de 3 ejes. Carpeta: `fw/lib/imu/`. I2C por el canal 5 del multiplexor, dirección 0x68 (AD0 en el GPIO26, que imu_begin() pone a 0).

Configuración fija, con constantes al inicio de imu.cpp: acelerómetro ±8 g, giróscopo ±2000 °/s.

```cpp
typedef struct {
    float ax, ay, az;   // aceleración en g (1 g = 9,81 m/s², la aceleración de la gravedad)
    float gx, gy, gz;   // velocidad de giro en °/s
} imu_data_t;

int  imu_begin();                     // 0 = OK, si no, código de error. Una vez en setup()
int  imu_update();                    // en cada loop(); nunca espera. 0 también si no hay dato nuevo
bool imu_new_data();                  // true si el último imu_update() leyó un dato nuevo
int  imu_get_state(imu_data_t *out);  // copia en *out el último dato válido. FW_ERR_BAD_PARAM si out es NULL
```


### Altímetro (`bar`, BMP581)

Librería ubicada en `fw/lib/bar/`. El sensor seleccionado es el SEN0667 (BMP581), conectado por I2C al canal 3 del multiplexor.

```cpp
int bar_begin();
int bar_update();
float bar_get_height_m();
float bar_get_pressure_hpa();
bool bar_is_calibrated();
```

- `bar_begin()`: inicializa el sensor, configura sus registros e inicia la calibración. Devuelve un código de estado.
- `bar_update()`: obtiene la primera presión válida tras `bar_begin()` como referencia fija P0 (altura relativa de 0 m). A partir de entonces actualiza la presión y calcula la altura relativa respecto a P0, sin esperas bloqueantes. Devuelve un código de estado.
- `bar_get_height_m()`: devuelve la última altura relativa válida en metros, o `NAN` si aún no está calibrado.
- `bar_get_pressure_hpa()`: devuelve la última presión válida en hPa, o `NAN` si todavía no hay ninguna.
- `bar_is_calibrated()`: devuelve `true` cuando se ha establecido P0 y `false` mientras la calibración está pendiente.

### Colisión frontal VL53L5CX `tof_front`

Carpeta: `fw/lib/tof_front/` · Nombre en la salida serie: `tof_front` · Placa: ESP32 DevKit (lógica a 3.3 V)

Configuración: 4x4 zonas (16), 30 Hz, medición continua. Constantes al inicio de `tof_front.cpp`.

```cpp
#define TOF_FRONT_NO_TARGET 0xFFFF         // marcador de "nada en rango"

int      tof_front_begin();                // 0 = OK, si no, código de error. Llamar una vez en setup().
                                        // Carga el firmware del sensor por I2C (puede tardar segundos). Nunca usa delay().
int      tof_front_update();               // llamar en cada loop(); nunca espera.
                                        // 0 = OK (también si aún no hay frame nuevo), si no, código de error
bool     tof_front_new_data();             // true si el último tof_front_update() produjo un frame válido nuevo
uint16_t tof_front_get_min_mm();           // obstáculo válido más cercano de todo el frame, en mm
                                        // TOF_FRONT_NO_TARGET si no hay nada en rango (rango válido: 20..4000 mm)
bool     tof_front_obstacle(uint16_t threshold_mm); // true si tof_front_get_min_mm() < threshold_mm
uint16_t tof_front_get_zone_mm(uint8_t row, uint8_t col); // row, col = 0..3; mm
                                        // TOF_FRONT_NO_TARGET si la zona no es válida o los índices están fuera de rango
uint32_t tof_front_get_age_ms();           // ms desde el último frame válido (para detectar datos obsoletos)
```

Unidades: milímetros en todo (enteros, sin floats).
Una zona es válida solo si el sensor indica target_status 5 o 9 y la distancia está entre 20 y 4000 mm.

### Colisión inferior (`tof_inf`, VL53L1X)

libreria ubicada en `fw/lib/tof_inf`

```cpp
int       tof_inf_begin();        // 0 = OK, otro número = error
int       tof_inf_update();       // llamar en cada loop(); nunca espera
bool      tof_inf_new_data();     // true si el último tof_inf_update() dejó una lectura nueva
uint16_t  tof_inf_get_mm();       // última distancia válida, en mm
```

Errores:

- `FW_ERR_NOT_FOUND`: el sensor no responde por I2C durante la inicialización o su identificador de modelo es distinto de `0xEACC`.
- `FW_ERR_TIMEOUT`: fallo de comunicación I2C durante el funcionamiento.
- `FW_ERR_BAD_DATA`: el `range_status` del driver ULD de ST es distinto de 0 o la distancia está fuera del rango válido del modo configurado. Un `range_status` de 0 indica una medición válida; no es el identificador del sensor. Se conserva la última lectura válida.

### Sensor de temperatura (`temp`, DS18B20)

Sonda DS18B20 (DFR0198) por 1-Wire, una sola en el bus. Librería en `fw/lib/temp/` Solo para ESP32. Se seleccionan las dependencias de Arduino `paulstoffregen/OneWire` y `milesburton/DallasTemperature` (nombres exactos en el registro de PlatformIO; véase `lib_deps` en el apartado Device). Configurar DallasTemperature con `setWaitForConversion(false)` para que la conversión sea no bloqueante.
``` cpp
int   temp_begin();      // 0 = OK, si no, código de error. Una vez en setup()
int   temp_update();     // en cada loop(); nunca espera. 0 también si aún no hay lectura nueva
bool  temp_new_data();   // true si el último temp_update() dejó una lectura nueva
float temp_get_c();      // última temperatura válida en °C (NAN si todavía no hay ninguna)
```

### Cámara (`camera`, enlace con la DSP)

La cámara no llega al ESP32: su vídeo entra en la DSP (LCDK C6748, RCA J9), que hace el procesado de imagen. El ESP32 recibe **solo el resultado** de cada imagen por UART y hace el resto del procesamiento. Librería en `fw/lib/camera/`; el firmware de la DSP va en `dsp/`.

Conexión (esquema de fabricación, commit d6fcc0c, conector J15 de la LCDK):

| Señal | ESP32 | DSP | Uso |
|---|---|---|---|
| UART TX | GPIO18 | J15 pin 17 (UART1_RXD) | ESP32 → DSP |
| UART RX | GPIO19 | J15 pin 19 (UART1_TXD) | DSP → ESP32 |
| DSP_SYNC | GPIO27 | J15 pin 16 (GPIO8[12]) | Reserva (pulso por imagen, si hiciera falta más precisión) |
| DSP_GPIO | GPIO13 | J15 pin 20 (GPIO8[10]) | Reserva |

En la LCDK hay que puentear R206 y R209 para que la UART llegue a la DSP. `Serial2.begin(115200, SERIAL_8N1, 19, 18)`.

Protocolo DSP → ESP32: el dron tiene que atravesar puertas marcadas con LEDs rojos. La DSP busca la puerta en la imagen y manda una línea ASCII por imagen, de 32 caracteres como mucho:

```
[dsp] t=<ms> cmd=<orden>
```

- `t`: marca de tiempo de la imagen, en ms desde que arrancó la DSP.
- `cmd`: una de estas 7 órdenes:

| `cmd` | Código | Significado |
|---|---|---|
| `quieto` | 0 | Centrado, o no se ve la puerta |
| `arriba` | 1 | La puerta está por encima |
| `abajo` | 2 | La puerta está por debajo |
| `izquierda` | 3 | La puerta está a la izquierda |
| `derecha` | 4 | La puerta está a la derecha |
| `adelante` | 5 | Centrado y lejos: avanzar |
| `atras` | 6 | Demasiado cerca: retroceder |

Ejemplo: `[dsp] t=15320 cmd=izquierda`.

```cpp
typedef enum {
  CAMERA_QUIETO = 0,
  CAMERA_ARRIBA = 1,
  CAMERA_ABAJO = 2,
  CAMERA_IZQUIERDA = 3,
  CAMERA_DERECHA = 4,
  CAMERA_ADELANTE = 5,
  CAMERA_ATRAS = 6
} camera_cmd_t;

typedef struct {
  uint32_t t_ms;      // marca de tiempo de la DSP
  int64_t t_rx_us;    // esp_timer_get_time() al recibir la linea en el ESP32
  camera_cmd_t cmd;   // orden de la DSP
} camera_result_t;

int  camera_begin();                          // Serial2. Una vez en setup()
int  camera_update();                         // lee lo que haya en la UART sin esperar
bool camera_new_data();                       // true si el ultimo update() completo una linea valida
int  camera_get_result(camera_result_t* out); // FW_ERR_BAD_PARAM si out es NULL, FW_ERR_NO_DATA si aun no hay
```

- `camera_update()` acumula bytes en un búfer fijo y solo publica líneas completas y bien formadas; una línea demasiado larga o mal formada se descarta con `FW_ERR_BAD_DATA`.

### Multiplexor adafruit2717 TCA9548A

Libreria ubicada en `fw/lib/common`. Dirección I2C: `0x70`


```cpp
    int mult_begin(void); //Inicializa y comprueba la comunicación con el multiplexor. Devuelve el código de error.
    int mult_select(int channel); //Recibe de argumento el canal que se quiere seleccionar (0-7) y devuelve el código de error.
    int mult_disable_all(void); // Desactiva todos los canales. devuelve el código de error.
```


## Error codes

Propuesta aceptada de códigos de error:

| Código | Nombre | Significado |
|---|---|---|
| 0 | `FW_ERR_OK` | correcto |
| 1 | `FW_ERR_NOT_FOUND` | dispositivo no encontrado en el bus I2C / ID incorrecto |
| 2 | `FW_ERR_TIMEOUT` | el dispositivo no responde dentro del plazo de comunicación |
| 3 | `FW_ERR_BAD_DATA` | el sensor devolvió datos inválidos o fuera de rango |
| 4 | `FW_ERR_BAD_PARAM` | argumento inválido (p. ej. índice de zona) |
| 5 | `FW_ERR_NOT_INIT` | no se llamó a `*_begin()` o falló |
| 6 | `FW_ERR_NO_DATA` | todavía no hay un dato o frame válido disponible; retorno inmediato |

Estos errores estarán recogidos en el archivo `fw/lib/common/fw_errors.h`

## Pin mapping

Placa: ESP32 DevKit M1. Los pines de I2C serán los mismos para los sensores que utilizan este protocolo, ya que se utilizara un multiplexor de I2C.

| component | signal | Arduino pin |
|---|---|---|
| Bus I2C (`mult`)| SCL | GPIO32 |
| Bus I2C (`mult`)| SDA | GPIO33 |
| IMU (`imu`)| SCL5, SDA5 | canal 5 del multiplexor |
| IMU (`imu`) | AD0 | GPIO26 |
| Altímetro (`bar`)| I2C | canal 3 del multiplexor|
| Colisión frontal (`tof_front`) | SDA7, SCL7 | canal 7 del multiplexor |
| Colisión frontal (`tof_front`) | LPn | GPIO14 |
| Colisión inferior (`tof_inf`)| I2C | canal 6 del multiplexor |
| Colisión inferior (`tof_inf`)| XSHUT| GPIO25|
| Temperatura (`temp`) | DQ (amarillo), con 4,7 kΩ a 3V3 | GPIO4 |
| Temperatura (`temp`) | VDD (rojo) | 3V3 |
| Temperatura (`temp`) | GND (negro) | GND |
| Cámara (`camera`) | UART TX → DSP | GPIO18 |
| Cámara (`camera`) | UART RX ← DSP | GPIO19 |
| Cámara (`camera`) | DSP_SYNC | GPIO27 (reserva) |
| Cámara (`camera`) | DSP_GPIO | GPIO13 (reserva) |

- SDA: I2C por defecto, compartido con otros sensores I2C (IMU, TOF inferior...).
- LPn: reservado: habilita/deshabilita el I2C del sensor.

## Serial format

- Banner, una vez al final de `setup()`: `FPF-LAB READY`. Lo imprime `main.cpp`, no las librerías.
- Una línea por actualización: `[<name>] key=value key=value`
- Errores: `[<name>] ERR code=<n>`
- 115200 baudios.

El resto de datos iria como se muestra en el siguiente ejemplo:

- En cada trama:
  -  `[tof_front] min_mm=850`
  -  `[imu] ax_g=0.5 ay_g=0.1 az_g=0.8 gx_deg_s=20 gy_deg_s=20 gz_deg_s=20`
  -  `[tof_inf] min_mm=850`
  -  `[bar] altura_m=1.5 pressure_hpa=1012.8`
  -  `[temp] temperatura_C=25`
  -  `[camera] t=15320 cmd=izquierda`
- En error: `[<name>] ERR code=2`

## Ejecución con FreeRTOS

El firmware corre sobre FreeRTOS (el que trae el ESP32 con Arduino). El reparto sale del documento de sincronización del Grupo 1: el I2C es un solo bus compartido por cuatro sensores a través del multiplexor, y los sensores no tienen pines de interrupción conectados, así que el ESP32 tiene que preguntarles si tienen dato.

| Tarea | Núcleo | Prioridad | Qué hace |
|---|---|---|---|
| `i2c` | 1 | 3 | La **única** que usa `Wire` y el multiplexor. Llama sin parar a `loop_tof_front()`, `loop_tof_inf()`, `loop_imu()`, `loop_bar()` y `loop_mult()`, en ese orden, y cede 1 ms en cada vuelta. |
| `temp` | 0 | 1 | Llama a `loop_temp()` cada 50 ms. La sonda va por 1-Wire en su propio pin y no comparte bus con nadie. |
| `loop()` de Arduino | 1 | 1 | `loop_camera()`: lee la UART de la DSP. |

Reglas que salen de este reparto:

- **Una sola tarea en el I2C.** Como solo la tarea `i2c` toca el bus, `mult_select()` + transferencia nunca se mezclan con las de otro sensor y no hace falta mutex. Ninguna otra tarea puede llamar a funciones I2C.
- **Cada componente se llama solo desde su tarea.** Sus `begin()` van en `setup()`, antes de crear las tareas; después, `update()` y los getters solo desde su tarea.
- **`update()` sigue sin esperar.** Cada librería decide con `millis()` si le toca medir y, si no, vuelve. Así un sensor lento no retrasa a los ToF. No se usa `delay()` ni `vTaskDelay()` dentro de `fw/lib`.
- **I2C a 400 kHz** (`Wire.setClock(400000)`, el máximo del TCA9548A).

### Frecuencia de cada sensor

Se mide más a menudo lo que importa para no chocar. Cada librería lleva su periodo como constante al inicio de su `.cpp`.

| Componente | Periodo | Por qué |
|---|---|---|
| `tof_front` | 33 ms (30 Hz, 4x4 zonas) | Colisión frontal: lo más urgente |
| `tof_inf` | 20 ms (50 Hz) | Colisión inferior y altura cerca del suelo |
| `imu` | 10 ms (100 Hz) | Actitud del dron |
| `bar` | 40 ms (25 Hz) | Altura; modo forzado, sin filtro IIR |
| `temp` | 5 s | La temperatura cambia despacio; la conversión ya tarda 750 ms |

La salida serie de cada componente se limita a una línea cada 100 ms como mucho, aunque lea más rápido; si no, a 115200 baudios el puerto se satura.

## Reglas

- **No bloqueante:** sin `delay()` ni `vTaskDelay()` en `fw/lib`. Las esperas usan `millis()` y retornan. Solo se permiten esperas necesarias dentro de las funciones `*_begin()`, llamadas desde `setup()`. Cada `delay()` permitido debe llevar el comentario `// setup-only` en la misma línea, por ejemplo: `delay(10);  // setup-only`. El job `lint` de CI rechaza cualquier `delay(` en `fw/lib/` que no tenga esa marca.
- **Búferes acotados:** arrays de tamaño fijo; nunca se escribe más allá del final.
- **Validar la entrada:** comprobar longitudes y valores antes de usar datos de un sensor.
- **Solo tu carpeta:** cada pareja solo cambia `fw/lib/<name>/` y su hueco en `main.cpp`.
- **Sincronización I2C:** Todas las librerías I2C seleccionan su canal del multiplexor mediante `mult_select()` antes de cada transferencia I2C al sensor, incluso si es el mismo canal que en la transferencia anterior.


### `tof_front`

- **Búferes acotados:** `uint16_t zones[16]` de tamaño fijo, sin `malloc`/`new`. Las transferencias I2C se parten en trozos que caben en el búfer de Wire.
- **Validar la entrada:** se comprueba el ID del dispositivo en `begin()`, el target_status y el rango de distancia de cada zona, y `row`/`col` antes de indexar.
- **Solo mi carpeta:** los cambios se limitan a `fw/lib/tof_front/` y a la ranura de `tof_front` en `main.cpp`.

### `temp`
- **No bloquea:** `update()` lanza la conversión y vuelve, sin `delay()`.
- **Buffers fijos:** 9 bytes de memoria y 8 de código ROM.
- **Valida los datos:** CRC y los 85 °C de arranque.
- **Solo se toca** `fw/lib/temp/` y el hueco de `temp` en `main.cpp`.
