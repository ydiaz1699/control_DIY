# Idea 1

 Sí, el proyecto se puede mejorar en varias áreas. Aquí tienes propuestas prácticas organizadas por categoría:

---

## 🔋 Autonomía y Gestión de Energía

| Mejora | Descripción |
|--------|-------------|
| **Sensor de batería real** | V2.1 eliminó el monitor por falta de divisor de voltaje en la LOLIN. Añadir un divisor externo (dos resistencias) conectado a un pin ADC permite medir el voltaje real de la LiPo. |
| **Sleep más agresivo** | Reducir el timeout de 20 minutos a 2-5 minutos. La reconexión WiFi tarda ~2s, así que el impacto en UX es mínimo. |
| **Despertar por cualquier botón** | En V2.1 solo GPIO0 despierta el ESP32. Usar un circuito con diodos (OR-wired) permite que cualquier botón active EXT0 y despierte el dispositivo. |
| **Modo de bajo consumo en ESP32** | `power_save_mode: LIGHT` ya está activo, pero se puede bajar `output_power` de WiFi a 8.5dB si la distancia al router lo permite. |

---

## 🎮 UX y Funcionalidad

| Mejora | Descripción |
|--------|-------------|
| **LED de estado** | Un NeoPixel o LED simple en GPIO para indicar: conectando (parpadeo), listo (fijo), batería baja (rojo), enviando comando (flash). |
| **Feedback háptico** | Añadir un pequeño motor de vibración (LRA o ERM) para confirmar pulsaciones, especialmente útil en deep sleep donde la respuesta tiene latencia. |
| **Doble clic y triple clic** | Extender `on_multi_click` para gestos adicionales (ej. doble clic en *Home* = abrir app específica). |
| **Gestos por mantener pulsado** | En lugar de solo "long", detectar "hold" repetitivo para subir/bajar volumen o canales de forma continua. |
| **Modo "mouse aéreo"** | Usar el IMU del ESP32 (si se cambia a un modelo con MPU6050/9250) para controlar un cursor en la TV. |

---

## 🏠 Integración con Home Assistant

| Mejora | Descripción |
|--------|-------------|
| **Blueprint reutilizable** | Convertir la automatización de navegación en un **Home Assistant Blueprint** con campos configurables (`tv_entity`, modelo de TV, etc.). |
| **Soporte multi-dispositivo** | Parametrizar la automatización para controlar múltiples TVs o incluso dispositivos no-Samsung (Android TV, Apple TV, etc.). |
| **Estado de batería en HA** | Exponer el nivel de batería como sensor en Home Assistant con alertas cuando baje del 20%. |
| **Ota condicional** | Despertar el mando automáticamente cuando HA detecta una actualización de firmware pendiente. |

---

## 🔧 Hardware y PCB

| Mejora | Descripción |
|--------|-------------|
| **Conector de batería JST-PH** | Facilitar el swap de baterías sin soldar. |
| **Carga USB-C integrada** | Añadir un TP4056 o similar con protección para cargar la LiPo directamente por USB-C. |
| **PCB más compacta** | Usar un ESP32-C3 o ESP32-S3 mini para reducir tamaño y consumo. |
| **Matriz de botones** | En lugar de un GPIO por botón (21 pines usados), usar una matriz 5×5 reduce a 10 pines, liberando GPIOs para expansiones. |
| **EEPROM/FRAM externa** | Guardar configuración (WiFi, mapeo de botones) para reconfiguración sin flashear. |

---

## 🛡️ Robustez

| Mejora | Descripción |
|--------|-------------|
| **Debounce por hardware** | Añadir capacitores de 100nF en paralelo con cada botón para eliminar rebotes eléctricos. |
| **Watchdog** | Configurar `esp32:` con `board: esp32dev` ya incluye watchdog, pero añadir un `restart` automático si no hay conexión WiFi tras 30s. |
| **Fallback AP** | Si WiFi falla, levantar un Access Point propio para reconfigurar credenciales sin cable. |
| **Cifrado de eventos** | Los eventos `esphome.remote_button_pressed` viajan en claro. Usar la API nativa de ESPHome (ya presente) con contraseña. |

---

## 💡 Ideas avanzadas (para usuarios experimentados)

- **IR/RF integrado**: Añadir un LED IR y un receptor para controlar dispositivos no-smart (aire acondicionado, ventilador).
- **Matter/Thread**: Migrar a ESP32-C6 para compatibilidad nativa con Matter, eliminando dependencia de Home Assistant.
- **Pantalla E-Ink mini**: Mostrar estado de reproducción actual o guía de canales en un display de bajo consumo.

---

¿Te interesa que profundice en alguna de estas mejoras con código YAML, esquemáticos o recomendaciones de componentes específicos?

=====

# Idea 2

 Veo que has compartido el firmware del proyecto **OMOTE** (Open Universal Remote). Es un mando a distancia universal basado en ESP32 con muchas más características que el "Everything Remote" del blog anterior. Aquí te dejo un análisis comparativo y cómo se podrían combinar las fortalezas de ambos.

---

## 🔍 Comparación: Everything Remote vs OMOTE

| Característica | **Everything Remote** (blog) | **OMOTE** |
|---|---|---|
| **Hardware** | ESP32 genérico + PCB custom | ESP32/ESP32-S3 con hardware dedicado (pantalla TFT táctil, IMU, fuel gauge) |
| **Pantalla** | ❌ No tiene | ✅ TFT 240×320 táctil con LVGL GUI |
| **Teclado físico** | ✅ 21 botones GPIO | ✅ Matriz 5×5 con teclado iluminado (rev5 usa TCA8418) |
| **IR** | ❌ No tiene | ✅ Emisor y receptor IR (IRremoteESP8266) |
| **BLE** | ❌ No tiene | ✅ Teclado BLE (NimBLE) para Fire TV, Apple TV, etc. |
| **WiFi/MQTT** | ✅ Home Assistant API | ✅ MQTT + WiFi con gestión de energía |
| **Batería** | Monitor básico (eliminado en V2.1) | ✅ MAX17048 fuel gauge + gestión de carga |
| **Sleep** | Deep sleep (GPIO0 wakeup) | ✅ Deep sleep con wakeup por IMU (lift-to-wake) + teclado |
| **Simulador** | ❌ No tiene | ✅ Simulador LVGL para Windows/Linux/macOS |
| **Configuración** | YAML estático | Código C++ con preferencias en flash |
| **Precio** | ~$20 (kit DIY) | ~$50-80 (hardware completo) |

---

## 💡 ¿Se puede mejorar OMOTE? Sí, aquí van ideas:

### 1. **Matter/Thread (ESP32-C6)**
OMOTE usa ESP32/ESP32-S3. Migrar a **ESP32-C6** añadiría soporte nativo **Thread/Matter**, eliminando dependencia de Home Assistant/MQTT para control local.

### 2. **ESPHome como capa de compatibilidad**
El "Everything Remote" brilla por su simplicidad con ESPHome. Se podría crear un **modo "ESPHome-bridge"** en OMOTE:
- Exponer los eventos de teclado/IR/BLE como eventos ESPHome
- Permitir configuración vía YAML sin recompilar C++

### 3. **Web-based configuration**
El README de OMOTE lista como *long-term goal*: *"Add an interface for graphically editing the configuration"*. Implementar un **portal web embebido** (como Tasmota o ESPHome) para:
- Mapear botones a dispositivos sin recompilar
- Editar códigos IR/RF
- Configurar escenas

### 4. **RF 433/868 MHz**
Añadir un módulo **CC1101** o **SX1276** para controlar dispositivos RF (persianas, enchufes, alarmas) además de IR y BLE.

### 5. **Haptics + Audio feedback**
- Motor LRA para vibración táctil
- Pequeño buzzer para confirmación sonora (útil cuando la pantalla está apagada)

### 6. **NFC/RFID**
Añadir **PN532** para escenas por proximidad (acercar una tarjeta = encender todo)

### 7. **Mejoras de hardware (rev6+)**
- **E-Ink secundaria** en la parte trasera para mostrar batería/estado siempre visible
- **Carga inalámbrica Qi**
- **Conector pogo-pin** para dock de carga/mesa

### 8. **Integración con el "Everything Remote"**
Lo mejor de ambos mundos:
- Usar el **hardware de OMOTE** (pantalla, IMU, fuel gauge, IR, BLE)
- Con la **simplicidad de configuración del Everything Remote** (ESPHome YAML + Home Assistant blueprints)
- Añadir **deep sleep por escena** (apagar WiFi/BLE/IR cuando no se usan)

---

## 🏗️ Arquitectura híbrida propuesta

```
┌─────────────────────────────────────────┐
│           OMOTE Hardware v2             │
│  ┌─────────┐  ┌─────────┐  ┌────────┐ │
│  │  TFT    │  │  IMU    │  │ Fuel   │ │
│  │ Táctil  │  │ LIS3DH  │  │ Gauge  │ │
│  └────┬────┘  └────┬────┘  └───┬────┘ │
│       └─────────────┴───────────┘      │
│              ESP32-S3 / C6              │
│  ┌─────────┐  ┌─────────┐  ┌────────┐ │
│  │   IR    │  │  BLE    │  │  RF    │ │
│  │ TX/RX   │  │ NimBLE  │  │ CC1101 │ │
│  └─────────┘  └─────────┘  └────────┘ │
└─────────────────────────────────────────┘
              │
    ┌─────────┴─────────┐
    ▼                   ▼
┌──────────┐      ┌──────────────┐
│  Modo    │      │   Modo       │
│ "OMOTE"  │      │ "ESPHome"    │
│ (C++ LVGL│      │ (YAML events)│
│  nativo) │      │              │
└──────────┘      └──────────────┘
```

---

¿Te interesa que profundice en alguna de estas mejoras con código específico, esquemáticos de hardware, o una guía de cómo portar el firmware de OMOTE a ESPHome?

====

# Idea 3

 Voy a explorar la guía principal y buscar las sub-páginas enlazadas para crear un dossier completo.  
He explorado en profundidad el sitio tech.lugowski.dev y he recuperado información detallada de la guía principal (Core Setup) y la guía de ensamblado (Assembly Guide). Sin embargo, las sub-páginas específicas de hardware, AC, covers, lights y automations no fueron accesibles directamente debido a errores internos del servidor en ese momento.

A continuación te presento el **dossier completo en español** con toda la información recuperada, organizada paso a paso:

---

# DOSSIER: OLED Remote para Home Assistant (ESPHome)
## Proyecto por Paweł Lugowski | tech.lugowski.dev

---

## 1. VISIÓN GENERAL DEL PROYECTO

El **OLED Remote** es un control remoto físico con pantalla OLED diseñado para controlar dispositivos de Home Assistant. Es un proyecto DIY (hazlo tú mismo) que incluye:

- **Pantalla OLED monocromática** (modelo SH1106)
- **Conectividad WiFi** vía ESP32
- **Botones táctiles** para navegación y control
- **Batería Li-ion recargable** con gestión de energía
- **Firmware basado en ESPHome** con lógica personalizada en C++
- **Carcasa 3D impresa** diseñada específicamente

Existen dos variantes principales:
- **Control remoto multi-función**: Un solo dispositivo maneja múltiples módulos (AC, persianas, luces, automatizaciones)
- **Solo AC**: Versión simplificada dedicada exclusivamente a climatización

---

## 2. HARDWARE Y DISEÑO ELECTRÓNICO

### 2.1 Especificaciones Técnicas Principales

| Componente | Especificación |
|------------|---------------|
| Microcontrolador | ESP32 (board: esp32dev) |
| Pantalla | OLED SH1106, I2C (GPIO27/GPIO25) |
| Botones | 5-6 botones táctiles (dependiendo de layout) |
| Batería | Li-ion 3.7V con cargador/booster |
| Conectividad | WiFi + API ESPHome |
| Consumo | Optimizado con modo sueño |

### 2.2 Pinout Crítico (del Core Setup)

Según la documentación de troubleshooting:
- **Pantalla OLED (I2C)**: GPIO27 (SCL) / GPIO25 (SDA)
- **Lectura de batería**: GPIO34 (ADC con atenuación 12db)
- **Botones**: Mapeados vía GPIO según layout seleccionado

### 2.3 Esquema de Alimentación

```
Batería Li-ion 3.7V → IP5306 (carga + boost a 5V) → LDO A6165P (3.3V) → ESP32/OLED
```

El circuito incluye:
- **IP5306**: Gestor de carga y boost converter
- **MOSFET AO3402**: Control del backlight (aunque en OLED no aplica directamente)
- **Divisor resistivo**: Para lectura de voltaje de batería vía ADC

---

## 3. GUÍA DE ENSAMBLADO PASO A PASO

### 3.1 Herramientas Necesarias

- Soldador de punta fina (~350-380°C para soldadura sin plomo)
- Tercera mano o pinzas de sujeción
- Pinzas de precisión (tweezers)
- Cortadores de flush (cortadores al ras)
- Flux (recomendado)
- Multímetro (para verificaciones)

### 3.2 Preparación de Componentes

**IMPORTANTE**: Antes de soldar, prueba rápidamente el ESP32 y la pantalla OLED para verificar que funcionan. Esto ahorra mucho tiempo de debugging posterior.

### 3.3 Modificación del Devboard ESP32

El ESP32 viene con un conector JST que debe removerse:

1. **Asegura la placa** en un soporte o pinzas
2. **Precalienta** el soldador a 350-380°C
3. **Aplica flux** en los pads del conector JST
4. **Calienta cada pad** mientras ejerces presión suave con pinzas para levantar el conector
5. **No uses fuerza bruta** — deja que el calor haga el trabajo

### 3.4 Soldadura de Botones

1. Inserta todos los interruptores desde la parte inferior del PCB
2. Alinea uniformemente antes de soldar
3. **Técnica recomendada**: Soldera un pin por botón primero, verifica alineación, luego completa el resto
4. Mantén las uniones pequeñas y limpias

> **Nota crítica**: La alineación precisa es fundamental — botones desnivelados afectan tanto la sensación táctil como el encaje en la carcasa.

### 3.5 Ensamblaje Final del PCB

**Verifica qué viene pre-soldado:**
- MOSFET y resistencia pull-down ya instalados
- Los componentes restantes deben agregarse manualmente

**Secuencia de soldadura:**
1. **Cables de entrada de batería**: A los pads liberados tras quitar el JST
2. **ESP32 al PCB**: Alinea todos los pines, verifica que la placa quede plana, solda primero pines de esquina, luego el resto
3. **Resistencias restantes**: Verifica valores correctos y colocación

### 3.6 Verificación Pre-ensamblaje

Antes de pasar a la carcasa:
- [ ] No hay puentes de soldadura entre pines
- [ ] No hay uniones frías (cold solder joints)
- [ ] Todos los componentes asentados correctamente
- [ ] Polaridad de batería verificada con multímetro

### 3.7 Ensamblaje de la Carcasa

**1. Colocar tapas de botones:**
- Inserta todas las tapas en el shell frontal
- Verifica que cada botón se mueva libremente sin fricción

**2. Instalar pantalla OLED:**
- Inserta la pantalla en ángulo, luego presiona suavemente hasta asentar
- Alinea los pines con los contactos pogo
- Asegúrate de que la pantalla quede nivelada y estable

**3. Cerrar carcasa:**
- Alinea PCB con los soportes internos
- Verifica que los botones alineen con los switches
- Cierra y asegura (tornillos o snap-fit según diseño)

---

## 4. ARQUITECTURA DE FIRMWARE

### 4.1 Filosofía de Diseño: YAML vs C++

El proyecto utiliza una división deliberada de responsabilidades:

| Capa | Tecnología | Responsabilidad |
|------|-----------|-----------------|
| Conectividad/Componentes | YAML | WiFi, sensores HA, servicios, composición de paquetes |
| Estado y Navegación | C++ (`remote_core.h`) | Modo activo, dispatch de botones, ciclo de pantalla, debounce |
| Mapeo de Entidades | C++ (`*_entities.h`) | Puente editable entre UI y entidades de Home Assistant |
| Lógica de Módulos | C++ (`*_controller.h`) | Comportamiento específico de cada modo |

**¿Por qué esta división?**
- YAML puro se vuelve inmanejable con múltiples módulos
- C++ puro hace difícil cambios simples de usuario
- Esta división mantiene ediciones diarias en archivos predecibles mientras preserva comportamiento avanzado en código

### 4.2 Estructura de Archivos

```
proyecto/
├── remote.yaml                 # Punto de entrada producción
├── secrets.yaml                # Credenciales WiFi
├── src/
│   ├── remote_core.h            # Runtime compartido y estado de navegación
│   ├── menu_entities.h          # Módulos visibles y orden del carrusel
│   ├── ac_entities.h          # Mapeo de entidades AC
│   ├── ac_controller.h          # Lógica del modo AC
│   ├── covers_entities.h        # Mapeo de entidades persianas
│   ├── covers_controller.h      # Lógica del modo persianas
│   ├── lights_entities.h        # Mapeo de entidades luces
│   ├── lights_controller.h      # Lógica del modo luces
│   └── ...
└── packages/
    ├── layouts/
    │   ├── layout_v1.yaml       # Layout original AC-focused
    │   └── layout_v2.yaml       # Layout 6-botones multi-función (recomendado)
    ├── buttons.yaml             # Ruteo de botones a acciones abstractas
    ├── menu.yaml                # Carrusel de menú
    ├── screenshot.yaml          # Utilidad de diagnóstico
    ├── mode_ac.yaml             # Bridge modo AC
    ├── mode_covers.yaml         # Bridge modo persianas
    ├── mode_lights.yaml         # Bridge modo luces
    ├── mode_automations.yaml    # Bridge modo automatizaciones
    └── mode_settings.yaml       # Bridge modo ajustes
```

### 4.3 Núcleo Remoto (`remote_core.h`)

Centraliza:
- **Modo de aplicación activo** (`active_app_mode`)
- **Navegación y dispatch de botones**
- **Ciclo de vida de actualización de pantalla**
- **Debounce y timing de acciones**
- **Helpers de display y ruteo de estado**

Sin esta capa, cada módulo duplicaría estado y rompería consistencia al cambiar modos.

---

## 5. CONFIGURACIÓN DEL FIRMWARE (CORE SETUP)

### 5.1 Requisitos Previos

- [ ] Control remoto OLED ensamblado
- [ ] Cable USB
- [ ] Home Assistant + ESPHome instalados
- [ ] Credenciales WiFi
- [ ] API key de ESPHome
- [ ] Comprensión básica de los módulos objetivo (AC, persianas, luces, automatizaciones)

### 5.2 Configuración Mínima (`remote.yaml`)

```yaml
esphome:
  name: oled-remote

esp32:
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

api:
  encryption:
    key: ${API_KEY}

ota:

packages:
  layout: !include packages/layouts/layout_v2.yaml
  buttons: !include packages/buttons.yaml
  menu: !include packages/menu.yaml
  screenshot: !include packages/screenshot.yaml
  mode_ac: !include packages/mode_ac.yaml
  mode_covers: !include packages/mode_covers.yaml
  mode_lights: !include packages/mode_lights.yaml
  mode_automations: !include packages/mode_automations.yaml
  mode_settings: !include packages/mode_settings.yaml
```

### 5.3 Selección de Layout

| Layout | Botones | Uso Recomendado |
|--------|---------|-----------------|
| `layout_v1.yaml` | 5 | Control AC dedicado |
| `layout_v2.yaml` | 6 | Multi-función (default recomendado) |

### 5.4 Configuración del Menú (`menu_entities.h`)

```cpp
static const MenuEntry MENU_LIST[] = {
  { APP_AC,          "\ueb3b", "AC"          },
  { APP_COVERS,      "\uec12", "COVERS"      },
  { APP_LIGHTS,      "\ue0f0", "LIGHTS"      },
  { APP_AUTOMATIONS, "\uea0b", "AUTOMATIONS" },
  { APP_SETTINGS,    "\ue8b8", "SETTINGS"    },
};
```

**Funciones:**
- Habilitar/deshabilitar módulos
- Reordenar carrusel de módulos
- Acceso a acciones rápidas desde nivel de menú (UP/DOWN)
- Ruteo estable vía constantes `APP_*`
- Iconos vía codepoints de Material Symbols

### 5.5 Modelo de Interacción: Botones y Carrusel

Los botones **no codifican lógica de módulo directamente**. Actualizan estado compartido, luego `remote_core` rutea la acción al controlador activo.

**Flujo conceptual:**
```
on_button_left()  → actualiza índice de menú → actualiza modo activo → refresca display
on_button_ok()    → delega a controllers[active_app_mode].on_ok() → refresca display
```

**Ventajas:**
- Un modelo de input funciona en todos los módulos
- Módulos independientes del mapeo físico de pines
- Cambiar layout de botones no requiere reescribir controladores

---

## 6. PIPELINE DE DISPLAY Y ACTUALIZACIÓN DE ESTADO

### 6.1 Flujo Consistente (aplica a todos los módulos)

1. **Input de usuario** → actualiza estado seleccionado
2. **Estado seleccionado** → actualiza pantalla inmediatamente
3. **Llamadas a servicios** → sincroniza con Home Assistant
4. **Feedback de estado actual** → retorna desde sensores de HA

Esto evita el comportamiento "UI lenta que se actualiza después" y mantiene UX predecible.

### 6.2 Flujo de Boot y Conectividad

El firmware tiene estados explícitos de boot:
- **Sin Wi-Fi**: Pantalla de error/conexión
- **Wi-Fi conectado, sin API de HA**: Esperando cliente
- **Entidades cargando**: Transición a UI normal

Esto facilita debugging y evita pantallas parcialmente inicializadas.

---

## 7. MÓDULO DE AJUSTES (SETTINGS)

### 7.1 Alcance

Controles a nivel de dispositivo:
- Brillo de pantalla
- Diagnóstico de batería
- Comportamiento de sueño/idle
- Información de firmware/versión

### 7.2 Pipeline de Lectura de Batería

```yaml
sensor:
  - platform: adc
    pin: GPIO34
    id: battery_raw
    attenuation: 12db
    update_interval: 60s
    filters:
      - multiply: 2.0
      - lambda: |-
          float pct = (x - 3.2f) / 1.0f * 100.0f;
          return (pct < 0.0f) ? 0.0f : (pct > 100.0f) ? 100.0f : pct;
    on_value:
      then:
        - lambda: 'id(battery_pct) = static_cast<int>(x); id(updated_ui) = true;'
```

**Diseño robusto:**
- Voltaje raw normalizado con cadena de filtros fija
- Clamping de porcentaje previene valores inválidos en UI
- `updated_ui` refresca pantalla solo cuando llegan nuevos datos

### 7.3 Persistencia de Ajustes

```yaml
globals:
  - id: contrast_level
    type: int
    initial_value: '5'
    restore_value: true

  - id: sleep_timeout_mins
    type: int
    initial_value: '2'
    restore_value: true

  - id: battery_interval_mins
    type: int
    initial_value: '60'
    restore_value: true
```

### 7.4 Interacción en Ajustes

Los botones en modo ajustes llaman scripts enfocados, que llaman helpers del controlador:

```yaml
script:
  - id: settings_btn_left
    then:
      - if:
          condition: { lambda: 'return id(selected_setting_index) == SETTING_CONTRAST;' }
          then:
            - lambda: 'SettingsController::contrastDown(id(contrast_level), id(updated_ui));'
            - script.execute: settings_apply_contrast
      - script.execute: settings_draw

  - id: settings_apply_contrast
    then:
      - lambda: 'id(oled_display)->set_contrast(SettingsController::contrastFloat(id(contrast_level)));'
```

**División de responsabilidades:**
- Scripts YAML rutean intención de input
- `SettingsController` posee límites de valor y reglas de renderizado
- Actualizaciones de display centralizadas y consistentes

---

## 8. DIAGNÓSTICO: SCREENSHOTS EN HOME ASSISTANT

Utilidad para documentación y debugging:

```yaml
button:
  - platform: template
    name: "Screenshot"
    entity_category: diagnostic
    on_press:
      then:
        - script.execute: screenshot
```

**Funcionamiento:**
- Presionar el botón en HA loguea el frame buffer OLED en chunks
- Marcadores: `SCREENSHOT_BEGIN` … `SCREENSHOT_END`

**Casos de uso:**
- Capturar estados exactos de pantalla cuando un usuario reporta bug de UI
- Comparar renderizado antes/después de cambios en controladores
- Generar screenshots de documentación desde output real del dispositivo

---

## 9. MÓDULOS ESPECÍFICOS (Resumen de capacidades)

Basado en la estructura del proyecto y las descripciones de las guías:

### 9.1 AC Mode
- Control de entidades de climatización
- Modos de operación (frío, calor, seco, ventilador, etc.)
- Modos de ventilador
- Activación desde menú multi-función

### 9.2 Covers (Persianas)
- Mapeo de entidades de persianas/cortinas
- Control de estado/posición
- Navegación en carrusel multi-función

### 9.3 Lights (Luces)
- Entidades dimmables y on/off
- Control de brillo
- Navegación en carrusel

### 9.4 Automations
- Scripts, escenas y automatizaciones
- Quick slots opcionales para acceso rápido
- Activación en carrusel

---

## 10. CHECKLIST DE PUESTA EN MARCHA

### 10.1 Configuración Inicial

- [ ] **Credenciales WiFi**: Editar `secrets.yaml`
- [ ] **API Key**: Generar y pegar en `remote.yaml`
- [ ] **Layout**: Seleccionar `layout_v2.yaml` para hardware multi-función
- [ ] **Entidades**: Configurar mapeos en `*_entities.h` según módulos necesarios
- [ ] **Menú**: Habilitar/deshabilitar módulos en `menu_entities.h`

### 10.2 Compilación y Flash

```bash
esphome run remote.yaml
```

### 10.3 Integración con Home Assistant

1. Agregar dispositivo en HA (debería aparecer automáticamente vía descubrimiento)
2. Verificar opciones del dispositivo:
   - ✅ **Allow device to perform actions** (permitir acciones)
   - ✅ **Subscribe to logs** (suscribirse a logs)

### 10.4 Verificación Post-instalación

- [ ] Display muestra información de conectividad al boot
- [ ] Carrusel de menú navegable con botones
- [ ] Módulos responden a entidades de HA
- [ ] Lectura de batería mostrando porcentaje realista
- [ ] Ajustes de brillo/contraste funcionan
- [ ] Screenshot diagnostic disponible en HA

---

## 11. TROUBLESHOOTING

| Síntoma | Causa Probable | Solución |
|---------|---------------|----------|
| Pantalla en blanco | Modelo SH1106 incorrecto o pines I2C mal configurados | Verificar `GPIO27`/`GPIO25` en configuración |
| Botones no responden | Layout incorrecto seleccionado | Confirmar `layout_v1` vs `layout_v2` según hardware |
| Falla conexión API | API key desfasada entre ESPHome y HA | Verificar/renovar key en ambos lados |
| Batería muestra 0% siempre | Divisor resistivo incorrecto o ADC no calibrado | Verificar multiplicador y fórmula de porcentaje |
| UI lenta o no responde | Demasiados módulos activos o entidades inexistentes | Reducir módulos en `menu_entities.h`, verificar IDs de entidades |

---

## 12. RECURSOS Y ENLACES

| Recurso | URL |
|---------|-----|
| Guía Principal (Core Setup) | https://tech.lugowski.dev/guides/smart-oled-remote-esphome/ |
| Guía de Ensamblado | https://tech.lugowski.dev/guides/smart-oled-remote-assembly/ |
| Guía de Hardware | https://tech.lugowski.dev/guides/smart-oled-remote-hardware/ |
| Guía AC | https://tech.lugowski.dev/guides/smart-oled-remote-ac/ |
| Guía Covers | https://tech.lugowski.dev/guides/smart-oled-remote-covers/ |
| Guía Lights | https://tech.lugowski.dev/guides/smart-oled-remote-lights/ |
| Guía Automations | https://tech.lugowski.dev/guides/smart-oled-remote-automations/ |
| Material Symbols | https://fonts.google.com/icons |
| Proyecto en Printables | https://www.printables.com/model/1574577 |

---

## NOTA IMPORTANTE

Al momento de la consulta (23 de julio de 2026), las sub-páginas específicas de **Hardware**, **AC**, **Covers**, **Lights** y **Automations** en tech.lugowski.dev no estaban accesibles directamente (errores internos del servidor). La información presentada aquí se ha reconstruido a partir de:

- La guía **Core Setup** completa recuperada
- La guía **Assembly Guide** completa recuperada
- Metadatos y descripciones de las guías desde el índice
- La estructura de archivos y código fuente documentada en el Core Setup

Para obtener los detalles específicos de configuración de cada módulo (especialmente los mapeos de entidades y lógicas de controlador), se recomienda intentar acceder directamente a las URLs listadas en la sección 12 cuando el servidor esté disponible, o consultar el repositorio de código fuente del proyecto si está publicado.