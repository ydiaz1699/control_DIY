#pragma once

#include <Arduino.h>
#include <vector>
#include <map>
#include <functional>

// ============================================================================
// Scene Manager - Manages scenes and quick actions
// Inspired by OMOTE scenes + Everything Remote HA events + homeThing groups
// ============================================================================

/// A scene definition
struct Scene {
    String name;
    String icon;        // LVGL symbol
    String haSceneId;   // HA entity ID (scene.xxx)
    std::vector<String> commands;  // Commands to execute in sequence
    bool showInCarousel;
    uint32_t color;     // Theme color for this scene
};

/// Scene Manager
class SceneManager {
public:
    static SceneManager& instance();

    void init();

    // Scene CRUD
    void addScene(const Scene& scene);
    void removeScene(const String& name);
    Scene* getScene(const String& name);
    std::vector<Scene*> getAllScenes();
    std::vector<Scene*> getCarouselScenes();

    // Scene activation
    void activateScene(const String& name);
    void activateSceneByIndex(int index);
    String getActiveSceneName() const { return activeScene_; }

    // NFC scene mapping
    void mapNFCTag(const String& tagId, const String& sceneName);
    void onNFCDetected(const String& tagId);

    // Callbacks
    using SceneCallback = std::function<void(const Scene&)>;
    void onSceneActivated(SceneCallback cb) { sceneActivatedCb_ = cb; }

private:
    SceneManager() = default;

    std::map<String, Scene> scenes_;
    std::map<String, String> nfcMappings_;  // tagId -> sceneName
    String activeScene_;
    SceneCallback sceneActivatedCb_ = nullptr;

    void loadScenesFromPreferences();
    void saveScenestoPreferences();
};
