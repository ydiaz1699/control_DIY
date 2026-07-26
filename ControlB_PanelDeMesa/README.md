# Control B — "Panel de Mesa" (Desk Panel)

## Fusión de: RemoteWebViewClient/Server + Controles Locales LVGL + ESP-NOW Sync

### Concepto
Panel fijo de sobremesa con pantalla táctil 4" 480x480 que muestra dashboards de Home Assistant en tiempo real (renderizados por servidor headless Chromium) y superpone controles táctiles locales rápidos.

---

## Arquitectura Dual

```
┌─────────────────────────────────────────────────────────────┐
│                  CONTROL B — PANEL DE MESA                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │         CAPA 1: Dashboard HA (WebView Stream)        │   │
│  │                                                      │   │
│  │  Servidor Node.js con Chromium headless renderiza    │   │
│  │  páginas HA → envía JPEG tiles via WebSocket →       │   │
│  │  ESP32 recibe y muestra en pantalla 480x480          │   │
│  │  Touch events se reenvían al servidor                │   │
│  └─────────────────────────────────────────────────────┘   │
│                         +                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │         CAPA 2: Widgets Locales (ESPHome LVGL)       │   │
│  │                                                      │   │
│  │  Panel lateral deslizable con:                       │   │
│  │  • Botones de escenas rápidas                        │   │
│  │  • Control de luces (slider brillo/color)            │   │
│  │  • Control media player (play/pause/vol)             │   │
│  │  • Estado del Mando Portátil (batería, modo)         │   │
│  │  • Clima/hora siempre visible                        │   │
│  └─────────────────────────────────────────────────────┘   │
│                         +                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │         CAPA 3: Comunicación ESP-NOW                 │   │
│  │                                                      │   │
│  │  Sincronización bidireccional con Control A:         │   │
│  │  • Escena cambiada en uno → actualiza el otro       │   │
│  │  • Estado de batería del mando visible en panel      │   │
│  │  • Comandos remotos entre dispositivos               │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  HARDWARE: ESP32-S3-4848S040 | 480x480 RGB666 | GT911 Touch│
│  SIEMPRE ENCHUFADO — Sin batería, sin deep sleep            │
└─────────────────────────────────────────────────────────────┘
```

---

## Hardware: Guition ESP32-S3-4848S040

| Componente | Especificación |
|---|---|
| MCU | ESP32-S3 (N16R8 — 16MB Flash, 8MB PSRAM) |
| Pantalla | 4" IPS 480x480 RGB666 (ST7701S driver) |
| Táctil | GT911 capacitivo (I2C) |
| RAM | 8MB PSRAM (Octal mode) |
| Conectividad | WiFi 802.11 b/g/n + BLE 5.0 |
| USB | USB-C (alimentación + programación) |
| Alimentación | 5V USB-C (siempre enchufado) |
| Expansión | I2C, SPI, GPIOs libres |
| Backlight | PWM (GPIO38) |

---

## Innovaciones sobre RemoteWebViewClient original

| Original | **Panel de Mesa (mejorado)** |
|---|---|
| Solo muestra dashboard HA | Dashboard HA + widgets locales superpuestos |
| Sin comunicación con otros dispositivos | ESP-NOW sync con Control A |
| Touch simple (tap) | Gestos avanzados: swipe, long-press, pinch |
| Sin feedback | Animaciones de transición + feedback visual |
| Una sola URL fija | Múltiples páginas navegables + auto-rotación |
| Sin modo idle | Reloj/clima cuando no hay interacción |
| Sin proximidad | VL53L0X: enciende pantalla al acercarse |
| Sin audio | Buzzer para notificaciones |

---

## Servidor (RemoteWebView Server)

El Panel de Mesa requiere el servidor RemoteWebView corriendo en tu red:

```bash
# Usando Docker (recomendado)
docker run -d --name rwv-server \
  -p 8081:8081 \
  -e HA_URL=http://homeassistant.local:8123 \
  strange-v/remote-webview-server:latest

# O manualmente (Node.js 20+)
cd server/
npm install
npm start
```

---

## Estructura ESPHome (Panel)

```
ControlB_PanelDeMesa/
├── README.md
├── panel_mesa.yaml          # Config principal ESPHome
├── secrets.yaml             # Credenciales (no se sube)
├── packages/
│   ├── display.yaml         # Configuración ST7701S 480x480
│   ├── touchscreen.yaml     # GT911 touch + gestures
│   ├── backlight.yaml       # PWM backlight control
│   ├── sensors.yaml         # Sensores HA + proximidad
│   └── espnow.yaml          # ESP-NOW sync component
├── components/
│   └── panel_widgets/       # Componente C++ custom para widgets
│       ├── __init__.py
│       ├── panel_widgets.h
│       └── panel_widgets.cpp
└── server/                  # RemoteWebView Server (Node.js)
    ├── Dockerfile
    ├── package.json
    ├── tsconfig.json
    └── src/
        ├── index.ts
        ├── protocol.ts
        ├── browser.ts
        ├── frameProcessor.ts
        └── config.ts
```

---

## Compilación y Despliegue

```bash
# Panel ESP32 (ESPHome)
esphome run panel_mesa.yaml

# Servidor (Docker)
cd server/
docker build -t rwv-server .
docker run -d -p 8081:8081 rwv-server

# O con Node.js directamente
cd server/ && npm install && npm start
```

---

## Licencia
GPL v3 — Basado en RemoteWebViewClient/Server (strange-v), con extensiones propias.
