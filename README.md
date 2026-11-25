# Firmware FPF-LAB

## Components

### IMU

6 datos de los grados de libertad:
- 3 aceleraciones que las pasaremos en g (número de veces la velocidad). En formato float (4 bytes).
- 3 velocidades de giro. representadas en º/s. En formato float (4 bytes).

Los 6 datos serán proporcionados por una única función que lee los registros correspondientes de la IMU y los esribe en la dirección de memoria indicada:

```cpp
int  get_state( float **target);
```

Regresa un int con 0 si no ha habido ningún error o el código de error en caso de que sí.

Para inicializar la IMU usaremos una función que también devuelve un int para el manejo de error. Como la IMU tiene varias configuraciones le pasaremos un int indicando que configuración vamos a usar. En caso de indicar una configuración que no existe devolverá un error.

```cpp
int  initialize(int config);
```

### Altímetro

Hay disponibilidad de 3 sensores que pueden funcionar como altímetro, la primera opción será utilizar el SEN0667 (BMP581) por sus mejores prestaciones para nuestra aplicación.

El sensor se comunica mediante el ESP32 mediante protocolo I2C.

```cpp
int altimeter_begin();
int altimeter_update();
float altimeter_get_height_m();
bool altimeter_is_calibrated();
```

- `altimeter_begin()`: incializa el sensor, configura sus registros y comienza el proceso de calibración. Devuelve un código de estado
- `altimeter_update()`: gestiona la calibración inicial y, una vez completada, actualiza las mediciones de presión y calcula la altura relativa cuando corresponde. No utiliza esperas bloqueantes y devuelve un código de estado
- `altimeter_get_height_m()`: devuelve la última altura medida calculada en metros
- `altimeter_is_calibrated()`: devuelve true cuando la calibración se ha completado y false mientras esta en proceso

### Colisión frontal (`tof_cf`, VL53L5CX)

Carpeta: `fw/lib/tof_cf/` · Nombre en la salida serie: `tof_cf` · Placa: ESP32 DevKit (lógica a 3.3 V)

Configuración: 4x4 zonas (16), 30 Hz, medición continua. Constantes al inicio de `tof_cf.cpp`.

```cpp
#define TOF_CF_NO_TARGET 0xFFFF         // marcador de "nada en rango"

int      tof_cf_begin();                // 0 = OK, si no, código de error. Llamar una vez en setup().
                                        // Carga el firmware del sensor por I2C (puede tardar segundos). Nunca usa delay().
int      tof_cf_update();               // llamar en cada loop(); nunca espera.
                                        // 0 = OK (también si aún no hay frame nuevo), si no, código de error
bool     tof_cf_new_data();             // true si el último tof_cf_update() produjo un frame válido nuevo
uint16_t tof_cf_get_min_mm();           // obstáculo válido más cercano de todo el frame, en mm
                                        // TOF_CF_NO_TARGET si no hay nada en rango (rango válido: 20..4000 mm)
bool     tof_cf_obstacle(uint16_t threshold_mm); // true si tof_cf_get_min_mm() < threshold_mm
uint16_t tof_cf_get_zone_mm(uint8_t row, uint8_t col); // row, col = 0..3; mm
                                        // TOF_CF_NO_TARGET si la zona no es válida o los índices están fuera de rango
uint32_t tof_cf_get_age_ms();           // ms desde el último frame válido (para detectar datos obsoletos)
```

Unidades: milímetros en todo (enteros, sin floats).
Una zona es válida solo si el sensor indica target_status 5 o 9 y la distancia está entre 20 y 4000 mm.

### Colisión inferior (VL53L1X)

```cpp
int       vl53l1x_begin();        // 0 = OK, otro número = error
int       vl53l1x_update();       // llamar en cada loop(); nunca espera
bool      vl53l1x_new_data();     // true si hay una medida nueva desde la última vez que se preguntó
uint16_t  vl53l1x_get_mm();       // última distancia válida, en mm
```

Errores: situación de no encontrado (el sensor no contesta por I2C o contesta con un identificador distinto de 0xEACC), fallo de comunicación (falla una lectura I2C a mitad de funcionamiento) y dato erróneo (el sensor contesta, pero el estado no es 9 o la distancia está fuera de 0 a 4000 mm).

### Sensor de temperatura (`temp`, DS18B20)

Sonda DS18B20 (DFR0198) por 1-Wire, una sola en el bus. Librería en `fw/lib/DS18B20/` (`OneWireBus` + `DS18B20`). Solo para ESP32.

```cpp
OneWireBus bus(4);       // GPIO de la línea DQ
DS18B20    sonda(bus);

DS18B20::Error begin(uint8_t resolucion = 12);  // 0 si va bien
bool  update();          // en cada loop(); no espera; true cuando hay lectura nueva
float temperatura();     // última temperatura en °C (NAN si ha fallado)
```

## Error codes

Propuesta aceptada de códigos de error:

| Código | Nombre | Significado |
|---|---|---|
| 0 | `ERR_OK` | correcto |
| 1 | `ERR_NOT_FOUND` | dispositivo no encontrado en el bus I2C / ID incorrecto |
| 2 | `ERR_TIMEOUT` | el dispositivo no responde / frame no listo a tiempo |
| 3 | `ERR_BAD_DATA` | el sensor devolvió datos inválidos o fuera de rango |
| 4 | `ERR_BAD_PARAM` | argumento inválido (p. ej. índice de zona) |
| 5 | `ERR_NOT_INIT` | no se llamó a `*_begin()` o falló |

## Pin mapping

Placa: ESP32 DevKit M1. Los pines de I2C serán los mismos para los sensores que utilizan este protocolo, ya que se utilizara un multiplexor de I2C.

| component | signal | Arduino pin |
|---|---|---|
| Bus I2C | SCL | GPIO32 |
| Bus I2C | SDA | GPIO33 |
| IMU | SCL5, SDA5 | canal 5 del multiplexor |
| Altímetro | I2C | canales 0, 2 y 3 del multiplexor |
| Colisión frontal (`tof_cf`) | SDA7, SCL7 | canal 7 del multiplexor |
| Colisión frontal (`tof_cf`) | LPn | GPIO14 |
| Colisión inferior | I2C | canal 6 del multiplexor |
| Temperatura (`temp`) | DQ (amarillo), con 4,7 kΩ a 3V3 | GPIO4 |
| Temperatura (`temp`) | VDD (rojo) | 3V3 |
| Temperatura (`temp`) | GND (negro) | GND |

- SDA: I2C por defecto, compartido con otros sensores I2C (IMU, TOF inferior...).
- LPn: reservado: habilita/deshabilita el I2C del sensor. Necesario para cambiar la dirección I2C si el TOF inferior comparte la 0x29.

## Serial format

- Banner, una vez al final de `setup()`: `FPF-LAB READY`. Lo imprime `main.cpp`, no las librerías.
- Una línea por actualización: `[<name>] key=value key=value`
- Errores: `[<name>] ERR code=<n>`
- 115200 baudios.

### `tof_cf`
- En cada frame nuevo (máx. 10 líneas/s): `[tof_cf] min_mm=850 valid=14` (`min_mm=-1` = sin objetivo, `valid` = número de zonas válidas de 16)
- En error (una vez por cambio de código, máx. 1/s): `[tof_cf] ERR code=2`
- No imprime nada más. El banner `FPF-LAB READY` lo imprime `main.cpp`, no esta librería.

### `temp`
- Una línea por lectura, una vez por segundo: `[temp] c=22.6875`
- Si falla: `[temp] ERR code=<n>`, por ejemplo `[temp] ERR code=1` con la sonda desconectada.

### IMU, Altímetro y Colisión inferior
- Pendiente: no hay línea de ejemplo en la #31.

## Rules

- **No bloqueante:** nunca `delay()` dentro de `fw/lib` (congela el resto de componentes).
- **Búferes acotados:** arrays de tamaño fijo; nunca se escribe más allá del final.
- **Validar la entrada:** comprobar longitudes y valores antes de usar datos de un sensor.
- **Solo tu carpeta:** cada pareja solo cambia `fw/lib/<name>/` y su hueco en `main.cpp`.

### `tof_cf`
- **No bloqueante:** sin `delay()`. Las esperas usan `millis()` y retornan. La impresión está limitada en frecuencia para que el búfer TX del Serial nunca se llene y bloquee `loop()`.
- **Búferes acotados:** `uint16_t zones[16]` de tamaño fijo, sin `malloc`/`new`. Las transferencias I2C se parten en trozos que caben en el búfer de Wire.
- **Validar la entrada:** se comprueba el ID del dispositivo en `begin()`, el target_status y el rango de distancia de cada zona, y `row`/`col` antes de indexar.
- **Solo mi carpeta:** los cambios se limitan a `fw/lib/tof_cf/` y a la ranura de `tof_cf` en `main.cpp`.

### `temp`
- **No bloquea:** `update()` lanza la conversión y vuelve, sin `delay()`.
- **Buffers fijos:** 9 bytes de memoria y 8 de código ROM.
- **Valida los datos:** CRC y los 85 °C de arranque.
- **Solo se toca** `fw/lib/DS18B20/` y el hueco de `temp` en `main.cpp`.
# dron_firmware
 
