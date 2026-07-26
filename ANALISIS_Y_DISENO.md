# Análisis Comparativo y Diseño de Controles Fusionados

## Fecha: Julio 2026
## Autor: Generado con Kiro AI

---

## PASO 1: TABLA COMPARATIVA DE LOS 5 PROYECTOS

| Característica | **OMOTE** | **homeThing** | **Everything Remote** | **RemoteWebView** | **OLED Remote** |
|---|---|---|---|---|---|
| **MCU** | ESP32 / ESP32-S3 | ESP32 (varios) | ESP32 genérico | ESP32-S3 | ESP32 |
| **Framework** | Arduino + PlatformIO | ESPHome | ESPHome | ESPHome (ESP-IDF) | ESPHome |
| **Pantalla** | TFT 2.8" 320x240 (ILI9341) | Varias (ST7789, AMOLED, M5Stack) | ❌ Sin pantalla | 4" 480x480 (ST7701S) | OLED SH1106 128x64 |
| **Táctil** | ✅ Sí (capacitivo) | ❌ No (encoder/botones) | ❌ No | ✅ GT911 capacitivo | ❌ No |
| **Botones** | ✅ Matriz 5x5 (TCA8418) | ✅ Encoder + botones | ✅ 21 GPIO directos | ❌ Solo touch | ✅ 5-6 botones |
| **IR TX/RX** | ✅ IRremoteESP8266 | ❌ No | ❌ No | ❌ No | ❌ No |
| **BLE** | ✅ NimBLE (teclado/media) | ❌ No | ❌ No | ❌ No | ❌ No |
| **WiFi** | ✅ + MQTT | ✅ ESPHome API | ✅ ESPHome API | ✅ WebSocket | ✅ ESPHome API |
| **HA Integration** | MQTT (manual) | Nativa (ESPHome) | Eventos ESPHome | WebSocket (dashboards) | Nativa (ESPHome) |
| **IMU** | ✅ LIS3DH (lift-to-wake) | ❌ No | ❌ No | ❌ No | ❌ No |
| **Batería** | ✅ MAX17048 fuel gauge | Depende del board | Monitor básico (ADC) | ❌ Siempre enchufado | ✅ ADC + IP5306 |
| **Deep Sleep** | ✅ (IMU + keypad wakeup) | ❌ No | ✅ (GPIO0 wakeup) | ❌ No | ✅ Limitado |
| **GUI Framework** | LVGL 8.3 | Display lambdas | Sin GUI | Sin GUI (WebView) | Display lambda + C++ |
| **Configuración** | Código C++ (recompilar) | YAML (no recompilar) | YAML (no recompilar) | YAML + server config | YAML + C++ headers |
| **Multi-click** | ❌ Básico | ❌ Básico | ✅ Completo (on_multi_click) | N/A | ✅ Parcial |
| **Media Player** | ✅ (Apple TV via BLE) | ✅ (Now Playing) | ❌ No | ✅ (via dashboard) | ❌ No |
| **Simulador** | ✅ SDL2 (Win/Linux/Mac) | ❌ No | ❌ No | Self-test mode | ❌ No |
| **Modularidad** | Media (scenes register) | Alta (apps/components) | Baja (monolítico) | Media (server+client) | Alta (packages/modes) |
| **Precio aprox.** | ~$50-80 | ~$15-40 | ~$20 | ~$25 (display) + server | ~$30 |

---

## PROS Y CONTRAS

### OMOTE
**Pros:** Hardware más completo (IR+BLE+WiFi+pantalla táctil+IMU+fuel gauge), GUI rica con LVGL, simulador para desarrollo, hardware open-source con PCB KiCad.
**Contras:** Configuración solo vía C++ (recompilar para cambios), curva de aprendizaje alta, precio mayor.

### homeThing
**Pros:** Soporte multi-pantalla, media player integrado, menú navegable con apps, componentes reutilizables, facilidad de ESPHome.
**Contras:** Sin IR ni BLE, sin táctil, dependiente de encoder/botones, documentación limitada.

### The Everything Remote
**Pros:** Extremadamente simple, deep sleep eficiente, multi-click elegante, barato, configuración 100% YAML, integración directa con automatizaciones HA.
**Contras:** Sin pantalla, sin feedback visual, limitado a WiFi, solo dispara eventos (lógica en HA).

### RemoteWebView (Client + Server)
**Pros:** Muestra cualquier dashboard de HA sin limitación, touch pass-through, múltiples clientes, muy visual.
**Contras:** Requiere servidor adicional (Docker/addon), depende de red, sin funcionalidad offline, latencia.

### OLED Remote
**Pros:** Arquitectura YAML+C++ muy limpia, carrusel de modos modulares, batería optimizada, bajo costo, fácil de personalizar entidades.
**Contras:** Pantalla pequeña monocromática, sin IR/BLE, solo WiFi, limitado visualmente.

---

## PASO 2: DISEÑO DE LOS DOS CONTROLES FUSIONADOS

### 🎮 CONTROL A — "Mando Portátil" (Basado en OMOTE)

#### Concepto
Un control remoto portátil universal que combina:
- El **hardware premium** de OMOTE (pantalla táctil, IR, BLE, IMU, fuel gauge)
- La **navegación por carrusel** de homeThing y OLED Remote
- El **deep sleep agresivo** y **multi-click** de Everything Remote
- Una **mini-vista de dashboard** inspirada en RemoteWebView
- La **arquitectura modular** (modos/modules) de OLED Remote

#### Qué toma de cada proyecto

| Proyecto | Elementos incorporados |
|----------|----------------------|
| OMOTE | Hardware completo, LVGL GUI, IR TX/RX, BLE keyboard, IMU lift-to-wake, MAX17048 fuel gauge, teclado TCA8418 |
| homeThing | Carrusel de modos, media player now playing, apps modulares |
| OLED Remote | Arquitectura core + módulos, entities editables, ruteo abstracto de botones |
| Everything Remote | Deep sleep con timer idle, multi-click (single/long/double), eventos HA |
| RemoteWebView | Concepto de "mini dashboard" como modo adicional en la pantalla |

#### Funcionalidades finales
1. **Home Screen**: Quick actions, estado rápido, acceso a modos
2. **Media Mode**: Now playing, control de volumen/seek, selección de fuente
3. **Climate Mode**: Temperatura, modos HVAC, ventilador
4. **Lights Mode**: Grupos de luces, sliders de brillo, toggle
5. **Covers Mode**: Persianas con slider de posición
6. **Scenes Mode**: Grid de escenas con activación rápida
7. **IR Learning**: Capturar y asignar códigos IR
8. **Settings**: WiFi, BLE pairing, batería, brillo, haptic, sync
9. **Idle Widget**: Reloj + clima cuando idle (antes de sleep)

---

### 🖥️ CONTROL B — "Panel de Mesa" (ESP32-S3-4848S040)

#### Concepto
Un panel fijo de sobremesa que combina:
- El **streaming de dashboards** de RemoteWebView (Chromium headless → tiles JPEG → WebSocket)
- **Controles táctiles nativos** como overlay complementario
- **Gestos avanzados** para navegación fluida
- **Sincronización** con Control A via MQTT

#### Qué toma de cada proyecto

| Proyecto | Elementos incorporados |
|----------|----------------------|
| RemoteWebView | Core completo: servidor headless, WebSocket, tiles JPEG, touch pass-through |
| homeThing | Concepto de apps/widgets nativos como complemento |
| OLED Remote | Carrusel de modos locales (escenas, luces, clima) |
| Everything Remote | Eventos HA para automatizaciones |
| OMOTE | Concepto de escenas predefinidas |

#### Modos de operación
1. **WebView Mode** (principal): Dashboard HA completo renderizado desde servidor
2. **Local Controls** (overlay): Widgets nativos sin servidor (escenas, sliders)
3. **Screensaver** (idle): Reloj + clima + estado de casa

---

## PASO 3: IDEAS CREATIVAS NUEVAS

### Ideas implementadas en ambos controles

| # | Idea | Control A | Control B | Descripción |
|---|------|:---------:|:---------:|-------------|
| 1 | **Gestos por IMU** | ✅ | — | Shake=play/pause, tilt=cambiar modo |
| 2 | **Gestos táctiles avanzados** | — | ✅ | Swipe, pinch, double-tap, long-press |
| 3 | **NFC Scene Trigger** | ✅ | — | Acercar tarjeta NFC = activar escena |
| 4 | **Feedback háptico** | ✅ | — | Motor LRA con patrones (click, error, success) |
| 5 | **Sincronización bidireccional** | ✅ | ✅ | Estado compartido vía MQTT entre ambos |
| 6 | **Temas dinámicos** | ✅ | ✅ | Cambio automático día/noche/película |
| 7 | **Widget idle inteligente** | ✅ | ✅ | Reloj + clima + estado de casa |
| 8 | **Notificaciones HA** | — | ✅ | Pop-ups con alertas de HA |
| 9 | **Multi-dashboard** | — | ✅ | Swipe entre dashboards (main, media, cámaras) |
| 10 | **IR Learning Mode** | ✅ | — | Aprender y asignar códigos IR |
| 11 | **Ambient brightness** | ✅ | ✅ | Brillo automático según hora/tema |
| 12 | **Heartbeat/Presencia** | ✅ | ✅ | Detección de si el otro control está online |
| 13 | **Control por voz** (preparado) | ✅ | — | Hardware listo (I2S mic), firmware extensible |
| 14 | **Screensaver inteligente** | — | ✅ | No solo reloj: muestra info útil del hogar |
| 15 | **Command relay** | ✅ | ✅ | Un control puede pedir al otro ejecutar acciones |

### Ideas futuras (no implementadas pero preparadas en la arquitectura)

- **Matter/Thread** vía ESP32-C6 (cambio de chip)
- **RF 433MHz** con módulo CC1101
- **E-Ink secundaria** en la parte trasera del mando
- **Carga inalámbrica Qi** para el mando
- **Reconocimiento de gestos por cámara** en el panel
- **Text-to-Speech** para confirmaciones audibles
- **Modo "mouse aéreo"** usando el IMU como puntero

---

## PASO 4: ESTRUCTURA DE IMPLEMENTACIÓN

```
control_DIY/
├── ANALISIS_Y_DISENO.md          ← Este documento
├── Ideas.md                       ← Ideas originales
├── README.md                      ← README del repo
│
├── ControlA_MandoPortatil/        ← CONTROL A
│   ├── README.md
│   ├── platformio.ini
│   ├── partitions.csv
│   └── src/
│       ├── main.cpp
│       ├── config.h
│       ├── core/                  (AppController, CommandRouter, PowerManager, SceneManager)
│       ├── hal/                   (Display, IR, BLE, IMU, Haptic, Battery, NFC)
│       ├── gui/                   (LVGL screens)
│       ├── connectivity/          (WiFi, MQTT, HA Bridge, Sync Protocol)
│       └── modules/               (Media, Climate, Lights, Covers, etc.)
│
├── ControlB_PanelMesa/            ← CONTROL B
│   ├── README.md
│   ├── panel_mesa.yaml            (ESPHome principal)
│   ├── secrets.yaml
│   ├── src/                       (C++ headers: gestures, themes, screensaver, notifications)
│   └── server/                    (Docker compose para RemoteWebView Server)
│
├── OMOTE-Firmware-main/           ← Referencia original
├── OMOTE-Hardware-main/           ← Referencia original
├── homeThing-main/                ← Referencia original
├── The-Everything-Remote-main/    ← Referencia original
├── RemoteWebViewClient-main/      ← Referencia original
└── RemoteWebViewServer-main/      ← Referencia original
```

---

## Cómo compilar y usar

### Control A (Mando Portátil)
```bash
cd ControlA_MandoPortatil/
# Editar src/config.h con tus credenciales
pio run -e esp32s3_omote
pio run -t upload
```

### Control B (Panel de Mesa)
```bash
cd ControlB_PanelMesa/
# Editar secrets.yaml con tus credenciales
esphome run panel_mesa.yaml

# Levantar el servidor
cd server/
docker-compose up -d
```

---

## Comunicación entre controles

Ambos controles se comunican vía **MQTT** usando el namespace `controldiy/sync/`:

| Topic | Dirección | Contenido |
|-------|-----------|-----------|
| `controldiy/sync/state` | Bidireccional | Estado general (modo activo, volumen, etc.) |
| `controldiy/sync/scene` | Bidireccional | Cambio de escena |
| `controldiy/sync/cmd` | Bidireccional | Relay de comandos |
| `controldiy/sync/heartbeat` | Bidireccional | Presencia (cada 30s) |
| `controldiy/sync/notification` | Panel → Mando | Notificaciones |

Formato de mensajes:
```json
{
  "type": 0,
  "from": "mando",
  "key": "scene",
  "value": "Ver TV",
  "ts": 123456789
}
```
