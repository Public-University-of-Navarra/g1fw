# Firmware FPF-LAB
## Device

El microcontrolador selecionado es el ESP32 DevKitM1. En platform.io se debe seleccionar `board = esp32dev`

## Componentes

### IMU (imu, ICM-40609-D)

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


### Altímetro

Libreria ubicada en `fw/lib/bar`

Hay disponibilidad de 3 sensores que pueden funcionar como altímetro, la primera opción será utilizar el SEN0667 (BMP581) por sus mejores prestaciones para nuestra aplicación.

El sensor se comunica mediante el ESP32 mediante protocolo I2C.

```cpp
int bar_begin();
int bar_update();
float bar_get_height_m();
bool bar_is_calibrated();
```

- `bar_begin()`: incializa el sensor, configura sus registros y comienza el proceso de calibración. Devuelve un código de estado
- `bar_update()`: gestiona la calibración inicial y, una vez completada, actualiza las mediciones de presión y calcula la altura relativa cuando corresponde. No utiliza esperas bloqueantes y devuelve un código de estado
- `bar_get_height_m()`: devuelve la última altura medida calculada en metros
- `bar_is_calibrated()`: devuelve true cuando la calibración se ha completado y false mientras esta en proceso

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

### Colisión inferior (VL53L1X)

libreria ubicada en `fw/lib/tof_inf`

```cpp
int       tof_inf_begin();        // 0 = OK, otro número = error
int       tof_inf_update();       // llamar en cada loop(); nunca espera
bool      tof_inf_new_data();     // true si el último temp_update() dejó una lectura nueva
uint16_t  tof_inf_get_mm();       // última distancia válida, en mm
```

Errores: situación de no encontrado (el sensor no contesta por I2C o contesta con un identificador distinto de 0, entonces devuelve `FW_ERR_NOT_FOUND`), fallo de comunicación (falla una lectura I2C a mitad de funcionamiento, `FW_ERR_TIMEOUT`)  y dato erróneo (`FW_ERR_BAD_DATA`).

### Sensor de temperatura DS18B20(`temp` )

Sonda DS18B20 (DFR0198) por 1-Wire, una sola en el bus. Librería en `fw/lib/temp/` (Además de librerias de terceros: `OneWireBus` + `DS18B20`). Solo para ESP32.
``` cpp
int   temp_begin();      // 0 = OK, si no, código de error. Una vez en setup()
int   temp_update();     // en cada loop(); nunca espera. 0 también si aún no hay lectura nueva
bool  temp_new_data();   // true si el último temp_update() dejó una lectura nueva
float temp_get_c();      // última temperatura válida en °C (NAN si todavía no hay ninguna) 
```

### Cámara

Libreria ubicada en `fw/lib/camera`

```cpp
int camera_Init();

int camera_DeInit();

int camera_Start();

int camera_Stop();

int camera_Update(); // Comprueba frame disponible y fin de transferencia DMA

int camera_GetFrame(uint8_t* buffer, size_t bufferSize, uint32_t timeoutMs);  //La función devuelve la imagen que está en memoria. 

int camera_GetFrameSize(size_t& size);

int camera_GetResolution(uint32_t& width, uint32_t& height);

int camera_GetSignalStatus(bool& signalDetected);
```

Todas las funciones devuelven uno de los códigos de error especificados. El formato de los frames puede cambiar según la cámara, asi que se dejan funciones para poder conocerlo. 

### Multiplexor adafruit2717 TCA9548A

Libreria ubicada en `fw/lib/common`. Dirección I2C: 112


```cpp
    int mult_begin(void) //Inicializa y comprueba la comunicación con el multiplexor. Devuelve el código de error.
    int mult_select(int channel); //Recibe de argumento el canal que se quiere seleccionar (0-7) y devuelve el código de error.
    int mult_disable_all(void); // Desactiva todos los canales. devuelve el código de error. 
```


## Error codes

Propuesta aceptada de códigos de error:

| Código | Nombre | Significado |
|---|---|---|
| 0 | `FW_ERR_OK` | correcto |
| 1 | `FW_ERR_NOT_FOUND` | dispositivo no encontrado en el bus I2C / ID incorrecto |
| 2 | `FW_ERR_TIMEOUT` | el dispositivo no responde / frame no listo a tiempo |
| 3 | `FW_ERR_BAD_DATA` | el sensor devolvió datos inválidos o fuera de rango |
| 4 | `FW_ERR_BAD_PARAM` | argumento inválido (p. ej. índice de zona) |
| 5 | `FW_ERR_NOT_INIT` | no se llamó a `*_begin()` o falló |

Estos errores estarán recogidos en el archivo `fw/lib/common/fw_errors.h` 

## Pin mapping

Placa: ESP32 DevKit M1. Los pines de I2C serán los mismos para los sensores que utilizan este protocolo, ya que se utilizara un multiplexor de I2C.

| component | signal | Arduino pin |
|---|---|---|
| Bus I2C (`mult`)| SCL | GPIO32 |
| Bus I2C (`mult`)| SDA | GPIO33 |
| IMU (`imu`)| SCL5, SDA5 | canal 5 del multiplexor |
| Altímetro (`bar`)| I2C | canal 3 del multiplexor|
| Colisión frontal (`tof_front`) | SDA7, SCL7 | canal 7 del multiplexor |
| Colisión frontal (`tof_front`) | LPn | GPIO14 |
| Colisión inferior (`tof_inf`)| I2C | canal 6 del multiplexor |
| Colisión inferior (`tof_inf`)| XSHUT| GPIO25|
| Temperatura (`temp`) | DQ (amarillo), con 4,7 kΩ a 3V3 | GPIO4 |
| Temperatura (`temp`) | VDD (rojo) | 3V3 |
| Temperatura (`temp`) | GND (negro) | GND |
|Cámara (`camera`)| Imagen analog| J9 (RCA del DSP)| 

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
  -  `[bar] altura_m=1.5`
  -  `[temp] temperatura_C=25`
  -  `[camera] is_any_frame=0` (0 es que se ha obtenido sin errores).
- En error: `[<name>] ERR code=2`

## Reglas

- **No bloqueante:** sin `delay()`. Las esperas usan `millis()` y retornan. Solo hay una excepción a esta regla, las funciones `begin` de cada libreria. 
- **Búferes acotados:** arrays de tamaño fijo; nunca se escribe más allá del final.
- **Validar la entrada:** comprobar longitudes y valores antes de usar datos de un sensor.
- **Solo tu carpeta:** cada pareja solo cambia `fw/lib/<name>/` y su hueco en `main.cpp`.
- **Sincronización I2C:** Todas las librerias seleccionan el canal del multiplexor antes de hacer transferencias. 


### `tof_front`

- **Búferes acotados:** `uint16_t zones[16]` de tamaño fijo, sin `malloc`/`new`. Las transferencias I2C se parten en trozos que caben en el búfer de Wire.
- **Validar la entrada:** se comprueba el ID del dispositivo en `begin()`, el target_status y el rango de distancia de cada zona, y `row`/`col` antes de indexar.
- **Solo mi carpeta:** los cambios se limitan a `fw/lib/tof_front/` y a la ranura de `tof_front` en `main.cpp`.

### `temp`
- **No bloquea:** `update()` lanza la conversión y vuelve, sin `delay()`.
- **Buffers fijos:** 9 bytes de memoria y 8 de código ROM.
- **Valida los datos:** CRC y los 85 °C de arranque.
- **Solo se toca** `fw/lib/temp/` y el hueco de `temp` en `main.cpp`. 

