#include "scene_manager.h"
#include "command_router.h"
#include "../config.h"
#include <Preferences.h>

extern void mqtt_publish(const char* topic, const char* payload);
extern void haptic_pulse(int durationMs);

// ============================================================================
// SceneManager Implementation
// ============================================================================

SceneManager& SceneManager::instance() {
    static SceneManager inst;
    return inst;
}

void SceneManager::init() {
    // Default scenes
    Scene tvScene;
    tvScene.name = "Ver TV";
    tvScene.icon = LV_SYMBOL_IMAGE;
    tvScene.haSceneId = "scene.ver_tv";
    tvScene.commands = {"samsung_tv/power"};
    tvScene.showInCarousel = true;
    tvScene.color = 0x2196F3;
    addScene(tvScene);

    Scene movieScene;
    movieScene.name = "Película";
    movieScene.icon = LV_SYMBOL_VIDEO;
    movieScene.haSceneId = "scene.pelicula";
    movieScene.commands = {};
    movieScene.showInCarousel = true;
    movieScene.color = 0x9C27B0;
    addScene(movieScene);

    Scene nightScene;
    nightScene.name = "Noche";
    nightScene.icon = LV_SYMBOL_CHARGE;  // Moon-like
    nightScene.haSceneId = "scene.buenas_noches";
    nightScene.commands = {};
    nightScene.showInCarousel = true;
    nightScene.color = 0x1A237E;
    addScene(nightScene);

    Scene allOffScene;
    allOffScene.name = "Todo Off";
    allOffScene.icon = LV_SYMBOL_POWER;
    allOffScene.haSceneId = "scene.todo_apagado";
    allOffScene.commands = {};
    allOffScene.showInCarousel = true;
    allOffScene.color = 0x424242;
    addScene(allOffScene);

    // Load custom scenes from NVS
    loadScenesFromPreferences();
}

void SceneManager::addScene(const Scene& scene) {
    scenes_[scene.name] = scene;
}

void SceneManager::removeScene(const String& name) {
    scenes_.erase(name);
}

Scene* SceneManager::getScene(const String& name) {
    auto it = scenes_.find(name);
    if (it != scenes_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<Scene*> SceneManager::getAllScenes() {
    std::vector<Scene*> result;
    for (auto& pair : scenes_) {
        result.push_back(&pair.second);
    }
    return result;
}

std::vector<Scene*> SceneManager::getCarouselScenes() {
    std::vector<Scene*> result;
    for (auto& pair : scenes_) {
        if (pair.second.showInCarousel) {
            result.push_back(&pair.second);
        }
    }
    return result;
}

void SceneManager::activateScene(const String& name) {
    Scene* scene = getScene(name);
    if (!scene) return;

    activeScene_ = name;

    // Call HA scene if configured
    if (scene->haSceneId.length() > 0) {
        String topic = String(MQTTConfig::DISCOVERY_PREFIX) + "/service/scene/turn_on";
        String payload = "{\"entity_id\":\"" + scene->haSceneId + "\"}";
        mqtt_publish(topic.c_str(), payload.c_str());
    }

    // Execute command sequence
    for (auto& cmd : scene->commands) {
        CommandRouter::instance().execute(cmd);
    }

    // Notify sync
    if (Features::SYNC_ENABLED) {
        String syncPayload = "{\"scene\":\"" + name + "\"}";
        mqtt_publish(SyncConfig::SYNC_TOPIC_SCENE, syncPayload.c_str());
    }

    // Haptic feedback
    if (Features::HAPTIC_ENABLED) {
        haptic_pulse(30);
    }

    // Callback
    if (sceneActivatedCb_) {
        sceneActivatedCb_(*scene);
    }
}

void SceneManager::activateSceneByIndex(int index) {
    auto scenes = getCarouselScenes();
    if (index >= 0 && index < (int)scenes.size()) {
        activateScene(scenes[index]->name);
    }
}

void SceneManager::mapNFCTag(const String& tagId, const String& sceneName) {
    nfcMappings_[tagId] = sceneName;
}

void SceneManager::onNFCDetected(const String& tagId) {
    auto it = nfcMappings_.find(tagId);
    if (it != nfcMappings_.end()) {
        activateScene(it->second);
    }
}

void SceneManager::loadScenesFromPreferences() {
    // Load from NVS - simplified for now
    Preferences prefs;
    if (prefs.begin("scenes", true)) {
        // Could store scene count and iterate
        prefs.end();
    }
}

void SceneManager::saveScenestoPreferences() {
    Preferences prefs;
    if (prefs.begin("scenes", false)) {
        // Save scene data
        prefs.end();
    }
}
