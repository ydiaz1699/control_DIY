#include "config_store.h"
#include <Preferences.h>

ConfigStore& ConfigStore::instance() {
    static ConfigStore inst;
    return inst;
}

// --- Command target <-> string (para que el JSON sea legible/editable a mano) ---

CommandTarget ConfigStore::targetFromString(const String& s) {
    if (s == "IR") return CommandTarget::IR;
    if (s == "BLE") return CommandTarget::BLE;
    if (s == "MQTT") return CommandTarget::MQTT;
    if (s == "HA_SERVICE") return CommandTarget::HA_SERVICE;
    if (s == "SYNC") return CommandTarget::SYNC;
    return CommandTarget::LOCAL;
}

String ConfigStore::targetToString(CommandTarget t) {
    switch (t) {
        case CommandTarget::IR: return "IR";
        case CommandTarget::BLE: return "BLE";
        case CommandTarget::MQTT: return "MQTT";
        case CommandTarget::HA_SERVICE: return "HA_SERVICE";
        case CommandTarget::SYNC: return "SYNC";
        default: return "LOCAL";
    }
}

// --- DeviceProfile <-> JSON ---

bool ConfigStore::deviceProfileFromJson(JsonObjectConst obj, DeviceProfile& out) {
    if (!obj.containsKey("name")) return false;

    out.name = obj["name"].as<String>();
    out.icon = obj["icon"] | "";

    JsonObjectConst commands = obj["commands"];
    for (JsonPairConst kv : commands) {
        Command cmd;
        JsonObjectConst c = kv.value().as<JsonObjectConst>();
        cmd.target      = targetFromString(c["target"] | "IR");
        cmd.action      = c["action"] | "";
        cmd.payload     = c["payload"] | "";
        cmd.irCode      = c["ir_code"] | (uint16_t)0;
        cmd.irProtocol  = c["ir_protocol"] | (uint8_t)1;
        cmd.repeat      = c["repeat"] | false;
        out.commands[String(kv.key().c_str())] = cmd;
    }
    return true;
}

void ConfigStore::deviceProfileToJson(const DeviceProfile& profile, JsonObject obj) {
    obj["name"] = profile.name;
    obj["icon"] = profile.icon;

    JsonObject commands = obj["commands"].to<JsonObject>();
    for (auto& kv : profile.commands) {
        JsonObject c = commands[kv.first].to<JsonObject>();
        c["target"]      = targetToString(kv.second.target);
        c["action"]      = kv.second.action;
        c["payload"]     = kv.second.payload;
        c["ir_code"]     = kv.second.irCode;
        c["ir_protocol"] = kv.second.irProtocol;
        c["repeat"]      = kv.second.repeat;
    }
}

// --- Scene <-> JSON ---

bool ConfigStore::sceneFromJson(JsonObjectConst obj, Scene& out) {
    if (!obj.containsKey("name")) return false;

    out.name           = obj["name"].as<String>();
    out.icon           = obj["icon"] | "";
    out.haSceneId      = obj["ha_scene_id"] | "";
    out.showInCarousel = obj["show_in_carousel"] | true;
    out.color          = obj["color"] | (uint32_t)0x424242;

    out.commands.clear();
    JsonArrayConst cmds = obj["commands"];
    for (JsonVariantConst v : cmds) {
        out.commands.push_back(v.as<String>());
    }
    return true;
}

void ConfigStore::sceneToJson(const Scene& scene, JsonObject obj) {
    obj["name"]              = scene.name;
    obj["icon"]              = scene.icon;
    obj["ha_scene_id"]       = scene.haSceneId;
    obj["show_in_carousel"]  = scene.showInCarousel;
    obj["color"]             = scene.color;

    JsonArray cmds = obj["commands"].to<JsonArray>();
    for (auto& c : scene.commands) cmds.add(c);
}

// --- Load ---

bool ConfigStore::loadDevices() {
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) return false;

    String json = prefs.getString(KEY_DEVICES, "");
    prefs.end();
    if (json.length() == 0) return false;

    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        Serial.println("[ConfigStore] Error parsing devices JSON, usando defaults");
        return false;
    }

    auto& router = CommandRouter::instance();
    for (JsonObjectConst devObj : doc["devices"].as<JsonArrayConst>()) {
        DeviceProfile profile;
        if (deviceProfileFromJson(devObj, profile)) {
            router.addDeviceProfile(profile);
        }
    }

    if (doc.containsKey("active_device")) {
        router.setActiveDevice(doc["active_device"].as<String>());
    }

    Serial.printf("[ConfigStore] %d devices cargados desde NVS\n",
                  doc["devices"].as<JsonArrayConst>().size());
    return true;
}

bool ConfigStore::loadScenes() {
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) return false;

    String json = prefs.getString(KEY_SCENES, "");
    prefs.end();
    if (json.length() == 0) return false;

    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        Serial.println("[ConfigStore] Error parsing scenes JSON, usando defaults");
        return false;
    }

    auto& sceneMgr = SceneManager::instance();
    for (JsonObjectConst sceneObj : doc["scenes"].as<JsonArrayConst>()) {
        Scene scene;
        if (sceneFromJson(sceneObj, scene)) {
            sceneMgr.addScene(scene);
        }
    }

    Serial.printf("[ConfigStore] %d scenes cargadas desde NVS\n",
                  doc["scenes"].as<JsonArrayConst>().size());
    return true;
}

// --- Save ---

bool ConfigStore::saveDevices(const std::map<String, DeviceProfile>& devices) {
    JsonDocument doc;
    JsonArray arr = doc["devices"].to<JsonArray>();
    for (auto& kv : devices) {
        deviceProfileToJson(kv.second, arr.add<JsonObject>());
    }

    String json;
    serializeJson(doc, json);

    if (json.length() > 15000) {
        Serial.println("[ConfigStore] WARN: devices JSON > 15KB, consider SPIFFS");
    }

    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) return false;
    size_t written = prefs.putString(KEY_DEVICES, json);
    prefs.end();

    Serial.printf("[ConfigStore] Devices guardados (%d bytes)\n", written);
    return written > 0;
}

bool ConfigStore::saveScenes(const std::map<String, Scene>& scenes) {
    JsonDocument doc;
    JsonArray arr = doc["scenes"].to<JsonArray>();
    for (auto& kv : scenes) {
        sceneToJson(kv.second, arr.add<JsonObject>());
    }

    String json;
    serializeJson(doc, json);

    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) return false;
    size_t written = prefs.putString(KEY_SCENES, json);
    prefs.end();

    Serial.printf("[ConfigStore] Scenes guardadas (%d bytes)\n", written);
    return written > 0;
}

// --- Export/Import (backup completo, idea #38 de Ideas.md) ---

String ConfigStore::exportConfigJson() {
    JsonDocument doc;

    // Export devices
    auto& router = CommandRouter::instance();
    JsonArray devArr = doc["devices"].to<JsonArray>();
    for (auto& name : router.getDeviceNames()) {
        DeviceProfile* dev = nullptr;
        // Need to get by name — set active temporarily to access
        String prevActive = router.getActiveDevice() ? router.getActiveDevice()->name : "";
        router.setActiveDevice(name);
        dev = router.getActiveDevice();
        if (dev) {
            deviceProfileToJson(*dev, devArr.add<JsonObject>());
        }
        if (prevActive.length() > 0) router.setActiveDevice(prevActive);
    }

    // Export scenes
    auto& sceneMgr = SceneManager::instance();
    JsonArray sceneArr = doc["scenes"].to<JsonArray>();
    for (auto* scene : sceneMgr.getAllScenes()) {
        sceneToJson(*scene, sceneArr.add<JsonObject>());
    }

    String json;
    serializeJsonPretty(doc, json);
    return json;
}

bool ConfigStore::importConfigJson(const String& json) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        Serial.println("[ConfigStore] Import failed: invalid JSON");
        return false;
    }

    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) return false;

    if (doc.containsKey("devices")) {
        String devJson;
        JsonDocument devDoc;
        devDoc["devices"] = doc["devices"];
        serializeJson(devDoc, devJson);
        prefs.putString(KEY_DEVICES, devJson);
        Serial.printf("[ConfigStore] Imported devices (%d bytes)\n", devJson.length());
    }

    if (doc.containsKey("scenes")) {
        String sceneJson;
        JsonDocument sceneDoc;
        sceneDoc["scenes"] = doc["scenes"];
        serializeJson(sceneDoc, sceneJson);
        prefs.putString(KEY_SCENES, sceneJson);
        Serial.printf("[ConfigStore] Imported scenes (%d bytes)\n", sceneJson.length());
    }

    prefs.end();

    Serial.println("[ConfigStore] Import complete — restart to apply");
    return true;
}
