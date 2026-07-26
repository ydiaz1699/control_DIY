#include "app_controller.h"
#include "../config.h"

// ============================================================================
// AppController Implementation
// ============================================================================

AppController& AppController::instance() {
    static AppController inst;
    return inst;
}

void AppController::init() {
    lastActivityMs_ = millis();
    lastUpdateMs_ = millis();
    currentMode_ = AppMode::HOME;

    // Initialize all registered modules
    for (int i = 0; i < (int)AppMode::MODE_COUNT; i++) {
        if (modules_[i] && modules_[i]->isEnabled()) {
            // Modules initialize their own state in onEnter()
        }
    }

    // Enter home mode
    if (modules_[(int)AppMode::HOME]) {
        modules_[(int)AppMode::HOME]->onEnter();
    }
}

void AppController::update() {
    unsigned long now = millis();
    unsigned long deltaMs = now - lastUpdateMs_;
    lastUpdateMs_ = now;

    // Update current module
    IAppModule* current = modules_[(int)currentMode_];
    if (current) {
        current->update(deltaMs);
    }

    // Check idle timeout for widget mode
    if (currentMode_ != AppMode::IDLE_WIDGET &&
        currentMode_ != AppMode::SETTINGS &&
        getIdleTimeMs() > GUIConfig::IDLE_WIDGET_DELAY) {
        // Switch to idle widget
        if (modules_[(int)AppMode::IDLE_WIDGET]) {
            setMode(AppMode::IDLE_WIDGET);
        }
    }
}

void AppController::setMode(AppMode mode) {
    if (mode == currentMode_) return;
    if ((int)mode >= (int)AppMode::MODE_COUNT) return;
    if (!modules_[(int)mode]) return;
    if (!modules_[(int)mode]->isEnabled()) return;

    AppMode oldMode = currentMode_;
    previousMode_ = currentMode_;

    // Exit current
    if (modules_[(int)currentMode_]) {
        modules_[(int)currentMode_]->onExit();
    }

    currentMode_ = mode;

    // Enter new
    modules_[(int)mode]->onEnter();

    // Notify callback
    if (modeChangeCb_) {
        modeChangeCb_(oldMode, mode);
    }

    resetIdleTimer();
}

void AppController::nextMode() {
    auto enabled = getEnabledModes();
    if (enabled.empty()) return;

    // Find current in enabled list
    int idx = 0;
    for (int i = 0; i < (int)enabled.size(); i++) {
        if (enabled[i] == currentMode_) {
            idx = i;
            break;
        }
    }
    idx = (idx + 1) % enabled.size();
    setMode(enabled[idx]);
}

void AppController::prevMode() {
    auto enabled = getEnabledModes();
    if (enabled.empty()) return;

    int idx = 0;
    for (int i = 0; i < (int)enabled.size(); i++) {
        if (enabled[i] == currentMode_) {
            idx = i;
            break;
        }
    }
    idx = (idx - 1 + enabled.size()) % enabled.size();
    setMode(enabled[idx]);
}

void AppController::goHome() {
    setMode(AppMode::HOME);
}

void AppController::handleButton(ButtonAction action, ButtonEventType type) {
    resetIdleTimer();

    // Wake from idle widget on any button
    if (currentMode_ == AppMode::IDLE_WIDGET && type == ButtonEventType::SINGLE_PRESS) {
        setMode(previousMode_);
        return;
    }

    // Global shortcuts
    if (type == ButtonEventType::SINGLE_PRESS) {
        switch (action) {
            case ButtonAction::HOME:
                goHome();
                return;
            case ButtonAction::MENU:
                nextMode();
                return;
            default:
                break;
        }
    }

    // Long press HOME = go to settings
    if (action == ButtonAction::HOME && type == ButtonEventType::LONG_PRESS) {
        setMode(AppMode::SETTINGS);
        return;
    }

    // Delegate to current module
    IAppModule* current = modules_[(int)currentMode_];
    if (current) {
        current->onButtonEvent(action, type);
    }
}

void AppController::handleTouch(int x, int y, int gesture) {
    resetIdleTimer();

    // Wake from idle
    if (currentMode_ == AppMode::IDLE_WIDGET) {
        setMode(previousMode_);
        return;
    }

    // Delegate
    IAppModule* current = modules_[(int)currentMode_];
    if (current) {
        current->onTouchEvent(x, y, gesture);
    }
}

void AppController::handleIMUGesture(int gestureType) {
    resetIdleTimer();

    // Gesture 1: Shake = toggle media play/pause
    // Gesture 2: Tilt left/right = prev/next mode
    // Gesture 3: Lift = wake
    switch (gestureType) {
        case 1: // Shake
            handleButton(ButtonAction::PLAY_PAUSE, ButtonEventType::SINGLE_PRESS);
            break;
        case 2: // Tilt left
            prevMode();
            break;
        case 3: // Tilt right
            nextMode();
            break;
        default:
            break;
    }
}

void AppController::registerModule(AppMode mode, IAppModule* module) {
    if ((int)mode < (int)AppMode::MODE_COUNT) {
        modules_[(int)mode] = module;
    }
}

IAppModule* AppController::getModule(AppMode mode) {
    if ((int)mode < (int)AppMode::MODE_COUNT) {
        return modules_[(int)mode];
    }
    return nullptr;
}

std::vector<AppMode> AppController::getEnabledModes() const {
    std::vector<AppMode> enabled;
    for (int i = 0; i < (int)AppMode::MODE_COUNT; i++) {
        if (modules_[i] && modules_[i]->isEnabled()) {
            // Skip idle widget from carousel
            if ((AppMode)i == AppMode::IDLE_WIDGET) continue;
            enabled.push_back((AppMode)i);
        }
    }
    return enabled;
}

void AppController::resetIdleTimer() {
    lastActivityMs_ = millis();
}

unsigned long AppController::getIdleTimeMs() const {
    return millis() - lastActivityMs_;
}

bool AppController::isIdle() const {
    return getIdleTimeMs() > PowerConfig::IDLE_TIMEOUT_MS;
}

AppController::SharedState AppController::getSharedState() const {
    SharedState state;
    state.activeMode = currentMode_;
    state.activeScene = "";
    state.volume = 0.5f;
    state.brightness = 1.0f;
    state.mediaPlaying = false;
    state.mediaTitle = "";
    state.mediaArtist = "";
    return state;
}

void AppController::applySharedState(const SharedState& state) {
    // Apply state received from sync (e.g., from Control B panel)
    // This allows both controls to stay synchronized
    if (state.activeScene.length() > 0) {
        // Trigger scene activation
    }
}
