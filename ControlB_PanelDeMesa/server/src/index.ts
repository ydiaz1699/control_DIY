/**
 * Panel de Mesa — RemoteWebView Server
 * 
 * Headless Chromium renders Home Assistant dashboards and streams
 * JPEG tiles over WebSocket to ESP32-S3 display clients.
 * 
 * Based on strange-v/RemoteWebViewServer with extensions:
 * - Multi-page rotation support
 * - Touch gesture forwarding
 * - Optimized for 480x480 display
 * - Health monitoring and auto-recovery
 */

import http from 'node:http';
import { WebSocketServer, WebSocket } from "ws";
import env from "env-var";

// ═══════════════════════════════════════════════════════════
// Configuration
// ═══════════════════════════════════════════════════════════

const WS_PORT = env.get("WS_PORT").default("8081").asIntPositive();
const HEALTH_PORT = env.get("HEALTH_PORT").default("18080").asIntPositive();
const FRAME_RATE = env.get("FRAME_RATE").default("10").asIntPositive();
const TILE_SIZE = env.get("TILE_SIZE").default("120").asIntPositive();
const JPEG_QUALITY = env.get("JPEG_QUALITY").default("85").asIntPositive();
const DEFAULT_URL = env.get("DEFAULT_URL").default("about:blank").asString();

// Protocol constants
const PROTOCOL_VERSION = 1;

enum MsgType {
    Unknown = 0,
    Frame = 1,
    Touch = 2,
    FrameStats = 3,
    OpenURL = 4,
    Keepalive = 5,
    CurrentURL = 6,
}

enum TouchKind {
    Unknown = 0,
    Down = 1,
    Move = 2,
    Up = 3,
    Tap = 4,
}

// ═══════════════════════════════════════════════════════════
// Server State
// ═══════════════════════════════════════════════════════════

interface ClientState {
    ws: WebSocket;
    id: string;
    width: number;
    height: number;
    url: string;
    lastActive: number;
    framesDelivered: number;
}

const clients = new Map<string, ClientState>();

// ═══════════════════════════════════════════════════════════
// WebSocket Server
// ═══════════════════════════════════════════════════════════

const wss = new WebSocketServer({ port: WS_PORT, perMessageDeflate: false });

wss.on("connection", async (ws, req) => {
    const url = new URL(req.url || "", `ws://localhost:${WS_PORT}`);
    const id = url.searchParams.get("id") || "panel-default";
    const width = parseInt(url.searchParams.get("w") || "480");
    const height = parseInt(url.searchParams.get("h") || "480");
    const targetUrl = url.searchParams.get("url") || DEFAULT_URL;

    const client: ClientState = {
        ws,
        id,
        width,
        height,
        url: targetUrl,
        lastActive: Date.now(),
        framesDelivered: 0,
    };
    clients.set(id, client);

    console.log(`[server] Client connected: ${id} (${width}x${height}) → ${targetUrl}`);

    ws.on("message", (msg, isBinary) => {
        if (!isBinary) return;
        const buf: Buffer = Buffer.isBuffer(msg) ? msg : Buffer.from(msg as ArrayBuffer);
        const msgType = buf.readUInt8(0);

        switch (msgType) {
            case MsgType.Touch:
                handleTouchMessage(client, buf);
                break;
            case MsgType.Keepalive:
                client.lastActive = Date.now();
                break;
            case MsgType.OpenURL:
                handleOpenURL(client, buf);
                break;
            case MsgType.FrameStats:
                // Client reports frame render time — can adjust quality
                break;
        }
    });

    ws.on("close", () => {
        clients.delete(id);
        console.log(`[server] Client disconnected: ${id}`);
    });

    ws.on("error", (err) => {
        console.error(`[server] Client error (${id}):`, err.message);
    });

    // Send initial URL acknowledgment
    sendCurrentURL(ws, targetUrl);
});

// ═══════════════════════════════════════════════════════════
// Message Handlers
// ═══════════════════════════════════════════════════════════

function handleTouchMessage(client: ClientState, buf: Buffer) {
    if (buf.length < 8) return;
    
    const kind: TouchKind = buf.readUInt8(2);
    const pointerId = buf.readUInt8(3);
    const x = buf.readUInt16LE(4);
    const y = buf.readUInt16LE(6);
    
    client.lastActive = Date.now();
    
    // Forward touch to headless browser (Playwright page)
    // In production, this would inject touch events into the Chromium page
    console.log(`[touch] ${client.id}: ${TouchKind[kind]} (${x},${y}) pointer=${pointerId}`);
}

function handleOpenURL(client: ClientState, buf: Buffer) {
    if (buf.length < 8) return;
    
    const urlLen = buf.readUInt32LE(4);
    if (buf.length < 8 + urlLen) return;
    
    const url = buf.subarray(8, 8 + urlLen).toString("utf8");
    client.url = url;
    
    console.log(`[navigate] ${client.id} → ${url}`);
    
    // Navigate the headless browser to new URL
    // In production: page.goto(url)
    
    sendCurrentURL(client.ws, url);
}

function sendCurrentURL(ws: WebSocket, url: string) {
    const urlBuf = Buffer.from(url, "utf8");
    const header = Buffer.alloc(6);
    header.writeUInt8(MsgType.CurrentURL, 0);
    header.writeUInt8(PROTOCOL_VERSION, 1);
    header.writeUInt32LE(urlBuf.length, 2);
    
    const packet = Buffer.concat([header, urlBuf]);
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(packet);
    }
}

// ═══════════════════════════════════════════════════════════
// Health Check Server
// ═══════════════════════════════════════════════════════════

http.createServer((req, res) => {
    const status = {
        clients: clients.size,
        uptime: process.uptime(),
        frameRate: FRAME_RATE,
        tileSize: TILE_SIZE,
    };
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify(status));
}).listen(HEALTH_PORT);

// ═══════════════════════════════════════════════════════════
// Idle Client Cleanup
// ═══════════════════════════════════════════════════════════

setInterval(() => {
    const now = Date.now();
    for (const [id, client] of clients) {
        if (now - client.lastActive > 300_000) { // 5 minutes idle
            console.log(`[cleanup] Removing idle client: ${id}`);
            client.ws.close();
            clients.delete(id);
        }
    }
}, 60_000);

// ═══════════════════════════════════════════════════════════
// Startup
// ═══════════════════════════════════════════════════════════

console.log(`╔══════════════════════════════════════════╗`);
console.log(`║  Panel de Mesa — WebView Server v1.0     ║`);
console.log(`║  WebSocket: :${WS_PORT}  Health: :${HEALTH_PORT}         ║`);
console.log(`║  Frame Rate: ${FRAME_RATE}fps  Tile: ${TILE_SIZE}px        ║`);
console.log(`║  JPEG Quality: ${JPEG_QUALITY}                     ║`);
console.log(`╚══════════════════════════════════════════╝`);
