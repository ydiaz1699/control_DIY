# Control DIY — Documento de Diseño

## Dos Controles Remotos Fusionados: Análisis, Diseño e Implementación

---

## PASO 1: TABLA COMPARATIVA DE PROYECTOS ORIGINALES

| Característica | OMOTE Firmware | OMOTE Hardware | homeThing | Everything Remote | RemoteWebView (Client+Server) | OLED Remote |
|---|---|---|---|---|---|---|
| **MCU** | ESP32 / ESP32-S3 | — | ESP32-S3 | ESP32 | ESP32-S3 | ESP32 |
| **Framework** | Arduino + PlatformIO | — | ESPHome | ESPHome | ESPHome (client) + Node.js (server) | ESPHome |
| **Pantalla** | TFT 2.8" 320x240 táctil | (diseño PCB) | TDisplay S3 320x170 | Ninguna | 4" 480x480 ST7701S | OLED SH1106 128x64 |
| **UI Library** | LVGL 8.3 | — | ESPHome Display API | — | JPEG tiles (server-rendered) | C++ directo |
| **IR TX/RX** | ✅ IRremoteESP8266 (multi-protocolo) | ✅ LED + receptor | ✅ IR blaster (GPIO13) | ❌ | ❌ | ❌ |
| **BLE** | ✅ NimBLE HID keyboard | — | ❌ | ❌ | ❌ | ❌ |
| **WiFi** | ✅ + MQTT (PubSubClient) | — | ✅ ESPHome API | ✅ ESPHome API | ✅ WebSocket | ✅ ESPHome API |
| **Batería** | MAX17048 fuel gauge | 2000mAh LiPo + protección | ADC + IP5306 | Deep sleep GPIO0 | N/A (USB alimentado) | ADC + IP5306 |
| **Sleep** | Deep sleep: IMU lift-to-wake + keypad | — | Timeout (2h) | Deep sleep 20min + EXT0 | ❌ (siempre on) | Timeout configurable |
| **IMU** | ✅ LIS3DH (lift-to-wake + motion) | ✅ en PCB | ❌ | ❌ | ❌ | ❌ |
| **Teclado** | TCA8418 (Rev5) / GPIO matrix | ✅ 5x5 key matrix | Rotary encoder + 5 botones | 21 GPIOs directos | Touch (GT911) | 6 botones |
| **Media Player** | ❌ (solo IR/BLE) | — | ✅ Sonos/Spotify/Roku | ❌ | Dashboard HA | ❌ |
| **Menú/Navegación** | Tabs LVGL | — | iPod scroll wheel | Ninguno | Dashboard HA completo | Carrusel de modos |
| **Escenas HA** | ✅ vía MQTT | — | ✅ ESPHome service | ✅ vía eventos | ✅ (dashboard nativo) | ✅ vía ESPHome |
| **Simulador** | ✅ SDL2 (Win/Linux/Mac) | — | ❌ | ❌ | ❌ | ❌ |
| **Costo aprox.** | $50-80 | PCB + componentes | $30-50 (TDisplay S3) | ~$20 | ~$25 (board) + servidor | ~$15 |

---

## PASO 2: DISEÑO DE LOS DOS CONTROLES FUSIONADOS

### 🎮 CONTROL A — "Mando Portátil"

| Aspecto | Decisión de Diseño |
|---|---|
| **Base hardware** | OMOTE PCB Rev5 (ESP32-S3, 16MB Flash, 8MB PSRAM) |
| **Pantalla** | 2.8" TFT 320x240 capacitiva (8-bit paralelo, LovyanGFX) |
| **UI Framework** | LVGL 9.x (upgrade de 8.3 por mejores gestos y rendimiento) |
| **Comunicaciones** | WiFi + MQTT + BLE keyboard + IR TX/RX + ESP-NOW |
| **Batería** | MAX17048 fuel gauge + 2000mAh LiPo |
| **Sleep** | Deep sleep con: IMU lift-to-wake + keypad INT + timeout configurable |
| **Navegación** | Carrusel (inspirado en OLED Remote) + swipe táctil + botones físicos |
| **Fuentes de código** | OMOTE (HAL, BLE, IR, sleep) + homeThing (media player) + OLED Remote (carrusel, modularidad) + Everything Remote (deep sleep events) |

### 🖥️ CONTROL B — "Panel de Mesa"

| Aspecto | Decisión de Diseño |
|---|---|
| **Base hardware** | ESP32-S3-4848S040 (16MB Flash, 8MB PSRAM) |
| **Pantalla** | 4" IPS 480x480 RGB666 (ST7701S) + GT911 touch capacitivo |
| **Framework** | ESPHome (esp-idf) + external component RemoteWebView |
| **Comunicaciones** | WiFi + ESPHome API + ESP-NOW |
| **Alimentación** | USB-C (siempre enchufado, sin batería) |
| **Modo display** | CAPA 1: Dashboard HA vía RemoteWebView JPEG tiles + CAPA 2: Widget overlay local |
| **Fuentes de código** | RemoteWebViewClient (tile rendering + touch) + panel widget C++ propio |

---

## PASO 3: INNOVACIONES CREATIVAS (No presentes en ningún proyecto original)

| # | Innovación | Descripción | Control |
|---|---|---|---|
| 1 | **ESP-NOW Sync** | Comunicación directa entre ambos controles sin router (<5ms latencia) | A + B |
| 2 | **Gesture Engine** | Swipe, pinch, long-press con feedback háptico | B (touch 480px) |
| 3 | **NFC Scene Trigger** | Acercar tarjeta NFC = activar escena predefinida | A |
| 4 | **Haptic Feedback** | Motor LRA con 7 patrones: click, long-press, error, confirm, mode change, startup, notification | A |
| 5 | **Dynamic Themes** | Cambia colores (day/night/movie) según hora del día automáticamente | A + B |
| 6 | **Idle Weather Widget** | Reloj + temperatura + condición climática cuando no se usa (antes de sleep) | A + B |
| 7 | **Voice Control** | Micrófono I2S con wake word local (placeholder para ESP-SR) | A |
| 8 | **IR Learning** | Aprende códigos IR de cualquier control existente y los almacena | A |
| 9 | **Media Player Universal** | Controla Spotify/Sonos/Chromecast desde pantalla táctil o botones | A |
| 10 | **Widget Overlay** | Panel deslizable superpuesto al dashboard HA con acciones rápidas | B |
| 11 | **Multi-page Dashboard** | Auto-rotación de páginas del dashboard + navegación por swipe | B |
| 12 | **Quick Actions** | Doble-tap en pantalla apagada/dim = acción rápida configurable | A + B |
| 13 | **Proximity Wake** | Sensor VL53L0X opcional: pantalla se enciende al acercar la mano | B |
| 14 | **Control A Status** | Panel B muestra batería y estado del mando portátil en widget | B |

---

## PASO 4: ESTRUCTURA DE IMPLEMENTACIÓN

```
control_DIY/
├── DESIGN_DOC.md                  ← Este archivo
├── Ideas.md                       ← Ideas originales del usuario
├── ControlA_MandoPortatil/        ← PROYECTO FUSIONADO 1
│   ├── README.md
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp
│       ├── config/
│       │   ├── pin_config.h       (GPIOs por revisión hardware)
│       │   └── user_config.h      (WiFi, MQTT, escenas, IR codes)
│       ├── hal/
│       │   ├── display_hal.*      (LovyanGFX + LVGL)
│       │   ├── battery_hal.*      (MAX17048 / ADC)
│       │   ├── imu_hal.*          (LIS3DH lift-to-wake)
│       │   ├── keypad_hal.*       (TCA8418 / GPIO matrix)
│       │   ├── haptic_hal.*       (LRA motor PWM)
│       │   └── ir_hal.*           (IRremoteESP8266 TX/RX + learning)
│       ├── comms/
│       │   ├── wifi_manager.*     (auto-connect + reconnect)
│       │   ├── mqtt_manager.*     (PubSubClient + HA topics)
│       │   ├── ble_keyboard.*     (NimBLE HID)
│       │   └── espnow_sync.*     (inter-device sync)
│       ├── gui/
│       │   ├── gui_manager.*      (LVGL screen management + carousel)
│       │   ├── theme_engine.*     (day/night/movie auto themes)
│       │   └── screens/
│       │       ├── screen_scenes.*    (scene grid)
│       │       ├── screen_media.*     (now playing)
│       │       ├── screen_ir_remote.* (per-device buttons)
│       │       ├── screen_smarthome.* (HA entities)
│       │       ├── screen_settings.*  (device config)
│       │       └── screen_idle.*      (clock + weather)
│       └── app/
│           ├── power_manager.*    (active→dim→sleep lifecycle)
│           ├── scene_manager.*    (MQTT scene activation + sync)
│           ├── media_player.*     (HA media_player control)
│           └── ir_controller.*    (IR device routing)
│
├── ControlB_PanelDeMesa/          ← PROYECTO FUSIONADO 2
│   ├── README.md
│   ├── panel_mesa.yaml            (config principal ESPHome)
│   ├── secrets.yaml.example
│   ├── packages/
│   │   ├── gestures.yaml          (gesture engine)
│   │   └── espnow_sync.yaml      (ESP-NOW receiver)
│   ├── components/
│   │   └── panel_widgets/         (custom ESPHome component)
│   │       ├── __init__.py
│   │       ├── panel_widgets.h
│   │       └── panel_widgets.cpp
│   └── server/                    (RemoteWebView server Node.js)
│       ├── Dockerfile
│       ├── package.json
│       ├── tsconfig.json
│       └── src/
│           └── index.ts
│
└── [proyectos originales para referencia]
    ├── OMOTE-Firmware-main/
    ├── OMOTE-Hardware-main/
    ├── homeThing-main/
    ├── The-Everything-Remote-main/
    ├── RemoteWebViewClient-main/
    └── RemoteWebViewServer-main/
```

---

## Cómo colaboran ambos controles

```
┌──────────────────┐                          ┌──────────────────┐
│  CONTROL A       │       ESP-NOW            │  CONTROL B       │
│  Mando Portátil  │◄──────────────────────►│  Panel de Mesa   │
│                  │   (sync bidireccional)   │                  │
│  • Escenas       │                          │  • Dashboard HA  │
│  • Media Player  │  ← Scene change sync →   │  • Widget overlay│
│  • IR TX/RX      │  ← State broadcast   →   │  • Gestos        │
│  • BLE Keyboard  │  ← Commands          →   │  • Proximidad    │
│  • Deep Sleep    │                          │  • Siempre ON    │
└──────┬───────────┘                          └──────┬───────────┘
       │                                              │
       │              WiFi + MQTT                     │
       └──────────────────┬───────────────────────────┘
                          │
                 ┌────────┴────────┐
                 │  HOME ASSISTANT  │
                 │  (MQTT Broker +  │
                 │   REST API)      │
                 └─────────────────┘
```

---

## Licencias y Créditos

- **OMOTE Community** — GPL v3 — Firmware + Hardware base
- **homeThing (landonr)** — ESPHome menu system y media player
- **Paweł Lugowski** — OLED Remote (arquitectura modular YAML+C++)
- **TheStockPot** — The Everything Remote (deep sleep + HA events)
- **strange-v** — RemoteWebView Client+Server (JPEG tile streaming)
- **Proyecto fusionado** — GPL v3 — Extensiones y combinación propia
