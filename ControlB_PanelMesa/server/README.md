# Remote WebView Server - Para Panel de Mesa

## Descripción

Servidor que renderiza dashboards de Home Assistant usando un navegador headless (Chromium)
y los transmite como tiles JPEG comprimidas vía WebSocket al Panel de Mesa (ESP32).

## Requisitos

- Docker y Docker Compose
- Red local con acceso a Home Assistant
- Panel de Mesa configurado y en la misma red

## Instalación Rápida

```bash
# 1. Configurar variables de entorno
cp .env.example .env
# Editar .env con la URL de tu HA

# 2. Levantar el servidor
docker-compose up -d

# 3. Verificar que está corriendo
docker-compose logs -f
```

## Configuración

Editar `.env`:

```
HA_URL=http://homeassistant.local:8123
HA_TOKEN=tu_long_lived_access_token
RWV_PORT=8081
```

## Alternativas de Despliegue

### 1. Docker (recomendado)
```bash
docker-compose up -d
```

### 2. Home Assistant Addon
Copiar la carpeta `hassio/` al directorio addons de HA.

### 3. Node.js directo
```bash
cd ../../RemoteWebViewServer-main
npm install
npm run build
npm start
```

## Notas Técnicas

- El servidor usa Playwright con Chromium para renderizar las páginas
- Las diferencias entre frames se detectan y solo se envían los tiles que cambiaron
- Soporta múltiples clientes simultáneos (hasta 4 displays)
- El panel táctil envía eventos de touch al servidor que los retransmite al navegador
- Configurado para 480x480 pixels para coincidir con la pantalla del panel
