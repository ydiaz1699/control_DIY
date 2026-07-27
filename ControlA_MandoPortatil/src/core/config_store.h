#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "command_router.h"
#include "scene_manager.h"

// ============================================================================
// ConfigStore - Carga/guarda DeviceProfiles y Scenes como JSON en NVS
// Permite editar mapeo de botones, comandos IR y escenas sin recompilar.
// ============================================================================

class ConfigStore {
public:
    static ConfigStore& instance();

    // Carga todo desde NVS. Devuelve false si no había nada guardado (primera vez).
    bool loadDevices();
    bool loadScenes();

    // Guarda el estado actual (se llama tras editar vía MQTT/portal web)
    bool saveDevices(const std::map<String, DeviceProfile>& devices);
    bool saveScenes(const std::map<String, Scene>& scenes);

    // Importa/exporta un blob JSON completo (para backup o "clonar mando")
    String exportConfigJson();
    bool importConfigJson(const String& json);

    // Parsers reutilizables (público para poder testear o reusar en el portal web)
    static bool deviceProfileFromJson(JsonObjectConst obj, DeviceProfile& out);
    static void deviceProfileToJson(const DeviceProfile& profile, JsonObject obj);
    static bool sceneFromJson(JsonObjectConst obj, Scene& out);
    static void sceneToJson(const Scene& scene, JsonObject obj);

private:
    ConfigStore() = default;
    static constexpr const char* NVS_NAMESPACE = "controldiy";
    static constexpr const char* KEY_DEVICES   = "devices_json";
    static constexpr const char* KEY_SCENES    = "scenes_json";

    static CommandTarget targetFromString(const String& s);
    static String targetToString(CommandTarget t);
};
