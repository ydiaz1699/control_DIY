# Control A — "Mando Portátil Universal"

## Basado en hardware OMOTE + firmware fusionado de lo mejor de 5 proyectos

---

## Descripción

Control remoto portátil universal con:
- **Pantalla táctil TFT 2.8" (320x240)** con interfaz LVGL
- **Botones físicos** (teclado matricial 5x5 con TCA8418)
- **IR TX/RX** para dispositivos legacy
- **BLE** para teclado/media (Fire TV, Apple TV, etc.)
- **WiFi + MQTT** para Home Assistant
- **IMU** (lift-to-wake)
- **Fuel Gauge MAX17048** (batería inteligente)
- **Deep sleep agresivo** con múltiples fuentes de wakeup

---

## Arquitectura de Firmware (Híbrida)

```
┌─────────────────────────────────────────────────────────────┐
│                    OMOTE Hardware Rev5                        │
│                      ESP32-S3 + PSRAM                        │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │   LVGL   │  │  IR      │  │  BLE     │  │  WiFi/   │   │
│  │   GUI    │  │  TX/RX   │  │  NimBLE  │  │  MQTT    │   │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘   │
│       │              │              │              │          │
│  ┌────┴──────────────┴──────────────┴──────────────┴─────┐  │
│  │              Unified Command Router                     │  │
│  │    (inspirado en OLED Remote remote_core.h)            │  │
│  └────┬──────────────┬──────────────┬──────────────┬─────┘  │
│       │              │              │              │          │
│  ┌────┴────┐   ┌────┴────┐   ┌────┴────┐   ┌────┴────┐    │
│  │  Scene  │   │ Media   │   │ Climate │   │ Smart   │    │
│  │ Manager │   │ Player  │   │ Control │   │ Home    │    │
│  └─────────┘   └─────────┘   └─────────┘   └─────────┘    │
│       │              │              │              │          │
│  ┌────┴──────────────┴──────────────┴──────────────┴─────┐  │
│  │              Home Assistant Bridge                      │  │
│  │    (eventos ESPHome-style + MQTT + API nativa)         │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Power Manager                            │    │
│  │  Deep Sleep · Lift-to-Wake · Battery Gauge · OTA     │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## Características Fusionadas

| Origen | Característica incorporada |
|--------|---------------------------|
| **OMOTE** | Hardware base, LVGL GUI, IR TX/RX, BLE keyboard, IMU, fuel gauge, MQTT |
| **homeThing** | Menú navegable por carrusel, media player integrado, grupos de luces, apps modulares |
| **OLED Remote** | Arquitectura YAML+C++ headers, modos modulares (AC/Covers/Lights/Automations), ruteo de botones abstracto |
| **Everything Remote** | Deep sleep agresivo, eventos HA, multi-click gestures, reset de idle timer |
| **RemoteWebView** | Mini-dashboard HA embebido como "modo vista" en la pantalla táctil |

---

## Ideas Creativas Nuevas

1. **Gestos por IMU**: Shake para cambiar escena, tilt para ajustar volumen
2. **NFC Tag Reader**: Tocar una tarjeta NFC = activar escena predefinida
3. **Feedback Háptico**: Motor LRA para confirmación táctil
4. **Widget Idle**: Reloj + clima + estado de casa cuando no se usa
5. **Modo Descubrimiento**: Auto-detecta dispositivos IR cercanos
6. **Sincronización con Panel de Mesa**: Estado compartido vía MQTT
7. **Temas Dinámicos**: Cambio automático día/noche según hora
8. **Control por Voz**: Botón dedicado + micrófono I2S para wake word local

---

## Hardware Requerido

- PCB OMOTE Rev5 (ESP32-S3 + PSRAM 8MB + Flash 16MB)
- Pantalla TFT ILI9341 2.8" 320x240 táctil
- TCA8418 Keypad Controller
- LIS3DH IMU
- MAX17048 Fuel Gauge
- IR LED + IR Receiver (TSOP38238)
- Batería LiPo 3.7V 2000mAh
- Motor vibración LRA (opcional)
- Módulo NFC PN532 (opcional)
- Micrófono I2S INMP441 (opcional)

---

## Build

```bash
# Requiere PlatformIO
cd ControlA_MandoPortatil
pio run -e esp32s3_omote
```

---

## Estructura del Proyecto

```
ControlA_MandoPortatil/
├── README.md
├── platformio.ini
├── partitions.csv
├── src/
│   ├── main.cpp
│   ├── config.h
│   ├── core/
│   │   ├── app_controller.h / .cpp
│   │   ├── command_router.h / .cpp
│   │   ├── scene_manager.h / .cpp
│   │   └── power_manager.h / .cpp
│   ├── hal/
│   │   ├── display_driver.h / .cpp
│   │   ├── touch_driver.h / .cpp
│   │   ├── keypad_driver.h / .cpp
│   │   ├── ir_driver.h / .cpp
│   │   ├── ble_driver.h / .cpp
│   │   ├── imu_driver.h / .cpp
│   │   ├── battery_driver.h / .cpp
│   │   ├── haptic_driver.h / .cpp
│   │   └── nfc_driver.h / .cpp
│   ├── gui/
│   │   ├── gui_manager.h / .cpp
│   │   ├── screens/
│   │   │   ├── screen_home.h / .cpp
│   │   │   ├── screen_media.h / .cpp
│   │   │   ├── screen_climate.h / .cpp
│   │   │   ├── screen_lights.h / .cpp
│   │   │   ├── screen_scenes.h / .cpp
│   │   │   ├── screen_settings.h / .cpp
│   │   │   └── screen_idle_widget.h / .cpp
│   │   └── themes/
│   │       ├── theme_dark.h
│   │       └── theme_light.h
│   ├── connectivity/
│   │   ├── wifi_manager.h / .cpp
│   │   ├── mqtt_client.h / .cpp
│   │   ├── ha_bridge.h / .cpp
│   │   └── sync_protocol.h / .cpp
│   └── modules/
│       ├── media_player.h / .cpp
│       ├── climate_control.h / .cpp
│       ├── light_control.h / .cpp
│       ├── cover_control.h / .cpp
│       ├── automation_trigger.h / .cpp
│       └── ir_learning.h / .cpp
└── data/
    └── (web config portal files)
```
