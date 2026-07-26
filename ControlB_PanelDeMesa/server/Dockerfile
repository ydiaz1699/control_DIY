# RemoteWebView Server — Headless Chromium Dashboard Renderer
# Renders HA dashboards and streams JPEG tiles to ESP32 displays
#
# Based on: https://github.com/strange-v/RemoteWebViewServer

FROM node:20-slim

# Install Chromium dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libwoff1 \
    libopus0 \
    libwebpdemux2 \
    libharfbuzz-icu0 \
    libenchant-2-2 \
    libgudev-1.0-0 \
    libsecret-1-0 \
    libhyphen0 \
    libgdk-pixbuf2.0-0 \
    libegl1 \
    libnotify4 \
    libxslt1.1 \
    libevent-2.1-7 \
    libgles2 \
    libvpx7 \
    libxcomposite1 \
    libatk1.0-0 \
    libatk-bridge2.0-0 \
    libepoxy0 \
    libgtk-3-0 \
    libpango-1.0-0 \
    libcairo2 \
    libgdk-pixbuf2.0-0 \
    libglib2.0-0 \
    libnss3 \
    libnspr4 \
    libdbus-1-3 \
    libatspi2.0-0 \
    libdrm2 \
    libxkbcommon0 \
    libx11-6 \
    libxcb1 \
    libxext6 \
    libxfixes3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy package files
COPY package*.json ./
COPY tsconfig.json ./

# Install dependencies and Playwright browsers
RUN npm ci --production=false
RUN npx playwright install chromium

# Copy source
COPY src/ ./src/

# Build TypeScript
RUN npm run build

# Environment variables
ENV WS_PORT=8081
ENV HEALTH_PORT=18080
ENV FRAME_RATE=10
ENV TILE_SIZE=120
ENV JPEG_QUALITY=85

EXPOSE 8081 18080

HEALTHCHECK --interval=30s --timeout=3s \
    CMD curl -f http://localhost:18080/ || exit 1

CMD ["node", "dist/index.js"]
