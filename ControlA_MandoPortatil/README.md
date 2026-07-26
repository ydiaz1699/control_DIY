# Control A — "Mando Portátil" (Portable Universal Remote)

## Fusión de: OMOTE + homeThing + OLED Remote + The-Everything-Remote

### Concepto
Control remoto portátil universal con pantalla táctil 2.8" + botones físicos, que combina lo mejor de 5 proyectos en un solo firmware optimizado.

---

## Tabla Comparativa de Proyectos Originales

| Característica | OMOTE | homeThing | OLED Remote | Everything Remote | **Control A (Fusión)** |
|---|---|---|---|---|---|
| MCU | ESP32/S3 | ESP32-S3 | ESP32 | ESP32 | **ESP32-S3 (PSRAM)** |
| Pantalla | TFT 2.8" táctil | TDisplay S3 | OLED SH1106 | Ninguna | **TFT 2.8" táctil** |
| UI Framework | LVGL 8.3 | ESPHome Display | C++ directo | - | **LVGL 9.x** |
| IR | TX + RX | TX | - | - | **TX + RX** |
| BLE | Teclado HID | - | - | - | **Teclado HID** |
| WiFi/MQTT | Sí | ESPHome API | ESPHome API | ESPHome API | **MQTT + HA REST** |
| Batería | MAX17048 gauge | ADC + IP5306 | ADC + IP5306 | Deep sleep | **MAX17048 + deep sleep** |
| Sleep | IMU lift-to-wake | Timeout | Timeout | GPIO wakeup | **IMU + GPIO + timeout** |
| Menú | Tabs LVGL | iPod scroll | Carrusel modos | - | **Carrusel + gestos táctiles** |
| Botones | Matriz TCA8418 | Encoder+5 | 6 botones | 21 GPIO | **TCA8418 + táctil** |
| Media Player | - | Sonos/Spotify | - | - | **Integrado** |
| Voz | - | Sí (mic) | - | - | **I2S mic integrado** |

---

## Hardware Base (OMOTE PCB Rev5)

- **MCU**: ESP32-S3 con PSRAM (16MB Flash + 8MB PSRAM)
- **Pantalla**: 2.8" TFT 320x240 capacitiva (8-bit paralelo, LovyanGFX)
- **IR**: TX (GPIO5) + RX con soporte multi-protocolo (IRremoteESP8266)
- **BLE**: NimBLE teclado HID para Fire TV, Apple TV, Android TV
- **WiFi**: MQTT + Home Assistant REST API
- **IMU**: LIS3DH (lift-to-wake + detección de movimiento)
- **Batería**: MAX17048 fuel gauge + 2000mAh LiPo
- **Teclado**: TCA8418 I2C keypad controller (matriz 5x5)
- **Extras añadidos**: Motor háptico LRA, buzzer, NFC (PN532 opcional)

---

## Arquitectura del Firmware

```
┌─────────────────────────────────────────────────────┐
│                    APLICACIÓN                         │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐ │
│  │ Escenas  │ │  Media   │ │    IR    │ │  Smart │ │
│  │ Carrusel │ │  Player  │ │ Learning │ │  Home  │ │
│  └──────────┘ └──────────┘ └──────────┘ └────────┘ │
├─────────────────────────────────────────────────────┤
│                   GUI (LVGL 9.x)                     │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐ │
│  │ Carousel │ │ Widgets  │ │ Gestures │ │ Themes │ │
│  └──────────┘ └──────────┘ └──────────┘ └────────┘ │
├─────────────────────────────────────────────────────┤
│              COMUNICACIÓN                            │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌────────┐ ┌────────┐ │
│  │ MQTT │ │ BLE  │ │  IR  │ │ESP-NOW │ │  NFC   │ │
│  └──────┘ └──────┘ └──────┘ └────────┘ └────────┘ │
├─────────────────────────────────────────────────────┤
│                    HAL                               │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌────────┐ ┌────────┐ │
│  │ TFT  │ │ IMU  │ │ Batt │ │Keypad  │ │Haptic  │ │
│  └──────┘ └──────┘ └──────┘ └────────┘ └────────┘ │
└─────────────────────────────────────────────────────┘
```

---

## Innovaciones Creativas (No presentes en ningún proyecto original)

1. **ESP-NOW Sync** — Sincroniza estado con el Panel de Mesa (Control B)
2. **Gestos avanzados** — Swipe para cambiar escena, long-press para favoritos
3. **NFC Scene Trigger** — Acercar tarjeta NFC = activar escena predefinida
4. **Feedback háptico** — Motor LRA confirma pulsaciones y cambios de modo
5. **Temas dinámicos** — Colores cambian según hora del día y actividad
6. **Widget idle** — Clima + hora cuando está inactivo (antes de dormir)
7. **Control por voz** — Micrófono I2S con wake word local
8. **IR Learning** — Aprende códigos IR de cualquier control existente
9. **Media Player universal** — Controla Spotify, Sonos, Chromecast desde el carrusel
10. **Quick Actions** — Doble-tap en pantalla apagada = acción rápida configurable

---

## Compilación

```bash
# Instalar PlatformIO CLI
pip install platformio

# Compilar para ESP32-S3 Rev5
pio run -e esp32s3

# Subir firmware
pio run -e esp32s3 -t upload

# Monitor serial
pio device monitor -b 115200
```

---

## Configuración

Editar `src/config/user_config.h` para personalizar:
- Credenciales WiFi/MQTT
- Dispositivos IR
- Entidades de Home Assistant
- Escenas y automatizaciones
- Temas de color

---

## Licencia
GPL v3 — Basado en trabajo de OMOTE Community, homeThing, Paweł Lugowski, TheStockPot.
