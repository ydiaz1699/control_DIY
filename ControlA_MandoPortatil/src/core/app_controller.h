#pragma once

#include <Arduino.h>
#include <functional>
#include <vector>
#include <string>

// ============================================================================
// App Controller - Central state machine and module coordinator
// Inspired by OLED Remote's remote_core.h and homeThing's menu system
// ============================================================================

/// Application modes (carousel items)
enum class AppMode : uint8_t {
    HOME = 0,       // Home screen with quick actions
    MEDIA,          // Media player control
    CLIMATE,        // AC / thermostat
    LIGHTS,         // Light groups and dimmers
    COVERS,         // Blinds / curtains
    SCENES,         // Scene activation
    SETTINGS,       // Device settings
    IR_LEARN,       // IR learning mode
    IDLE_WIDGET,    // Clock + weather widget
    MODE_COUNT      // Sentinel
};

/// Button actions (abstracted from physical keys)
enum class ButtonAction : uint8_t {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    OK,
    BACK,
    HOME,
    MENU,
    VOL_UP,
    VOL_DOWN,
    CH_UP,
    CH_DOWN,
    MUTE,
    POWER,
    PLAY_PAUSE,
    // Extended
    SHORTCUT_1,
    SHORTCUT_2,
    SHORTCUT_3,
    SHORTCUT_4,
    ACTION_COUNT
};

/// Button event type
enum class ButtonEventType : uint8_t {
    SINGLE_PRESS,
    LONG_PRESS,
    DOUBLE_PRESS,
    HOLD_REPEAT
};

/// Module interface - each screen/mode implements this
class IAppModule {
public:
    virtual ~IAppModule() = default;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void onButtonEvent(ButtonAction action, ButtonEventType type) = 0;
    virtual void onTouchEvent(int x, int y, int gesture) = 0;
    virtual void update(unsigned long deltaMs) = 0;
    virtual const char* getName() const = 0;
    virtual const char* getIcon() const = 0;  // LVGL symbol or Material icon codepoint
    virtual bool isEnabled() const { return true; }
};

/// App Controller - manages mode transitions and delegates input
class AppController {
public:
    static AppController& instance();

    void init();
    void update();

    // Mode management
    void setMode(AppMode mode);
    AppMode getMode() const { return currentMode_; }
    void nextMode();
    void prevMode();
    void goHome();

    // Input dispatch (from keypad/touch drivers)
    void handleButton(ButtonAction action, ButtonEventType type);
    void handleTouch(int x, int y, int gesture);
    void handleIMUGesture(int gestureType);

    // Module registration
    void registerModule(AppMode mode, IAppModule* module);
    IAppModule* getModule(AppMode mode);
    std::vector<AppMode> getEnabledModes() const;

    // Activity tracking (for power management)
    void resetIdleTimer();
    unsigned long getIdleTimeMs() const;
    bool isIdle() const;

    // State sharing (sync with Control B)
    struct SharedState {
        AppMode activeMode;
        String activeScene;
        float volume;
        float brightness;
        bool mediaPlaying;
        String mediaTitle;
        String mediaArtist;
    };
    SharedState getSharedState() const;
    void applySharedState(const SharedState& state);

    // Callbacks
    using ModeChangeCallback = std::function<void(AppMode oldMode, AppMode newMode)>;
    void onModeChange(ModeChangeCallback cb) { modeChangeCb_ = cb; }

private:
    AppController() = default;
    AppController(const AppController&) = delete;

    AppMode currentMode_ = AppMode::HOME;
    AppMode previousMode_ = AppMode::HOME;
    IAppModule* modules_[(int)AppMode::MODE_COUNT] = {nullptr};
    unsigned long lastActivityMs_ = 0;
    unsigned long lastUpdateMs_ = 0;
    ModeChangeCallback modeChangeCb_ = nullptr;
};
