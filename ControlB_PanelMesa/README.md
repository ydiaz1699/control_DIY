# Control B — "Panel de Mesa"

## Dashboard HA en tiempo real + controles táctiles locales

---

## Descripción

Panel fijo de sobremesa con:
- **Pantalla 4" (480x480) táctil capacitiva** (Guition ESP32-S3-4848S040 o similar)
- **Dashboard Home Assistant** renderizado en servidor headless Chromium y transmitido vía WebSocket
- **Controles táctiles locales** como complemento (widgets, carrusel de escenas)
- **Siempre enchufado** (sin batería, sin deep sleep)
- **Gestos avanzados** (swipe, pinch, long press)
- **Sincronización** con Control A vía MQTT

---

## Arquitectura

```
┌─────────────────────────────────────────────────────────────────┐
│                    ESP32-S3-4848S040                              │
│              (ESP32-S3 + PSRAM 8MB + Flash 16MB)                 │
│              Pantalla 4" 480x480 RGB666 + Touch GT911            │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌────────────────────────────┐  ┌────────────────────────────┐ │
│  │   Remote WebView Mode      │  │   Local Controls Mode      │ │
│  │                            │  │                            │ │
│  │  ┌──────────────────────┐  │  │  ┌──────────────────────┐ │ │
│  │  │ WebSocket Client     │  │  │  │ Scene Grid (3x3)     │ │ │
│  │  │ ← JPEG Tiles         │  │  │  │ Quick Actions        │ │ │
│  │  │ → Touch Events       │  │  │  │ Climate Widget       │ │ │
│  │  │ ← Current URL        │  │  │  │ Media Now Playing    │ │ │
│  │  └──────────────────────┘  │  │  │ Light Sliders        │ │ │
│  │                            │  │  │ Clock + Weather       │ │ │
│  │  Server (Docker/Addon):    │  │  └──────────────────────┘ │ │
│  │  Playwright + Chromium     │  │                            │ │
│  │  → Renders HA Dashboard    │  │  Communication:            │ │
│  │  → Encodes JPEG tiles      │  │  MQTT ↔ Home Assistant    │ │
│  │  → Streams via WebSocket   │  │  Sync ↔ Control A (Mando) │ │
│  └────────────────────────────┘  └────────────────────────────┘ │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                    Gesture Engine                          │   │
│  │  Swipe L/R: cambiar página | Swipe D: panel rápido       │   │
│  │  Long press: editar | Pinch: zoom (WebView)              │   │
│  │  Double tap: toggle pantalla completa                     │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │           Theme Engine (dinámico según hora)              │   │
│  │  06:00-18:00: Light theme | 18:00-06:00: Dark theme      │   │
│  │  Custom per-scene colors                                  │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
         │
         │ WiFi
         ▼
┌─────────────────────────────┐
│   RemoteWebView Server      │
│   (Docker / HA Addon)       │
│                             │
│   Playwright + Chromium     │
│   WebSocket Broadcaster     │
│   JPEG Encoder (Sharp)      │
└─────────────────────────────┘
```

---

## Dos Modos de Operación

### Modo 1: WebView Dashboard (principal)
- Muestra un dashboard de HA renderizado desde el servidor
- Touch pass-through al servidor (clicks, scroll)
- Navegación de páginas vía swipe o botones en servidor
- Ideal para dashboards complejos con gráficos, cámaras, mapas

### Modo 2: Controles Locales (complemento)
- Widgets nativos en el ESP32 sin depender del servidor
- Grid de escenas (3x3) con iconos y colores
- Sliders de luces y clima
- Now Playing con controles de media
- Reloj/clima como screensaver
- Se activa con swipe down o botón dedicado en la UI

---

## Características Nuevas (no en proyectos originales)

1. **Motor de Gestos Avanzado**: Swipe multidireccional, pinch-to-zoom, long press con feedback visual
2. **Tema Dinámico**: Cambia automáticamente según hora del día o escena activa
3. **Screensaver Inteligente**: Muestra reloj + clima + estado de casa cuando idle
4. **Notificaciones de HA**: Pop-ups con notificaciones de Home Assistant
5. **Sincronización Bidireccional**: Con Control A (Mando Portátil) vía MQTT
6. **Multi-Dashboard**: Swipe entre múltiples dashboards de HA
7. **Overlay de Controles Rápidos**: Swipe down para controles locales sobre WebView
8. **Ambient Display**: Brillo automático según hora (no sensor LDR por defecto)

---

## Hardware Requerido

- **Guition ESP32-S3-4848S040** (o compatible):
  - ESP32-S3 con PSRAM 8MB OPI
  - Flash 16MB
  - Pantalla 4" 480x480 IPS (ST7701S RGB666)
  - Touch capacitivo GT911
  - Backlight PWM
  - USB-C (alimentación + programación)
- **Fuente USB-C 5V 2A** (siempre enchufado)
- **RemoteWebView Server** corriendo en Docker o HA Addon

---

## Instalación

### 1. Servidor (Docker)
```bash
cd ControlB_PanelMesa/server/
docker-compose up -d
```

### 2. Firmware (ESPHome)
```bash
# Con ESPHome CLI
esphome run panel_mesa.yaml
```

### 3. Configuración
Editar `secrets.yaml` con WiFi y server IP.

---

## Estructura del Proyecto

```
ControlB_PanelMesa/
├── README.md
├── panel_mesa.yaml              # ESPHome principal
├── secrets.yaml                 # Credenciales (template)
├── packages/
│   ├── board_4848s040.yaml      # Configuración de hardware
│   ├── display_st7701s.yaml     # Display RGB666
│   ├── touchscreen_gt911.yaml   # Touch capacitivo
│   ├── webview.yaml             # Remote WebView component
│   ├── local_controls.yaml      # Widgets nativos
│   ├── gestures.yaml            # Motor de gestos
│   ├── sync.yaml                # Protocolo sync con Control A
│   ├── notifications.yaml       # Notificaciones HA
│   └── screensaver.yaml         # Screensaver inteligente
├── src/
│   ├── gesture_engine.h         # Detección de gestos avanzados
│   ├── theme_engine.h           # Temas dinámicos
│   ├── local_ui.h               # UI nativa (escenas, sliders)
│   ├── screensaver.h            # Clock + weather widget
│   └── notification_overlay.h   # Pop-up de notificaciones
├── fonts/
│   ├── material_icons_24.h
│   ├── roboto_16.h
│   └── roboto_24.h
└── server/
    ├── docker-compose.yml       # Docker compose para el servidor
    ├── .env                     # Variables de entorno
    └── README.md                # Instrucciones del servidor
```
